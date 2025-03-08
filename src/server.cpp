#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <stdexcept>

#include <tl/expected.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "sim_server.grpc.pb.h"
#include "world_state.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using sim_server::Metadata;
using sim_server::SimulationResultResponse;
using sim_server::StartSimulationRequest;
using sim_server::StateService;
using sim_server::Vector1D;
using sim_server::Vector2D;
using sim_server::Vector3D;

class StateServiceImpl final : public StateService::Service
{
public:
    static const uint64_t kDefaultSimulationTimeoutSeconds = 3;

    Status InitWorldState(ServerContext *context, const sim_server::InitializeRequest *request,
                          sim_server::WorldStateResponse *reply) override
    {
        size_t x_max = request->dimensions().x_max();
        size_t y_max = request->dimensions().y_max();
        size_t z_max = request->dimensions().z_max();

        tl::expected<std::tuple<uint64_t, Grid3D>, std::string> world_state = InitWorldStateInternal(x_max, y_max, z_max);
        const auto &[id, grid] = *world_state;

        // Serialize the generated world state into the response
        ConvertGrid3DToProto(grid, *reply->mutable_state());
        reply->mutable_metadata()->set_state_id(id);
        reply->mutable_metadata()->set_step(0);
        reply->mutable_metadata()->set_status("World state initialized");

        return Status::OK;
    }

    Status StepWorldStateForward(ServerContext *context, const sim_server::StepRequest *request,
                                 sim_server::WorldStateResponse *reply) override
    {
        Bitset128 rule;
        std::memcpy(&rule, request->rule().data(), sizeof(Bitset128));

        const uint64_t world_state_id = request->world_state_id();
        Grid3D updated_world_state = StepWorldStateForwardInternal(world_state_id, rule);

        // Save the updated state
        set_world_state_by_id(world_state_id, updated_world_state);

        // Serialize the updated world state into the response
        ConvertGrid3DToProto(updated_world_state, *reply->mutable_state());
        reply->mutable_metadata()->set_state_id(world_state_id);
        reply->mutable_metadata()->set_step(get_step_by_world_state_id(world_state_id) + 1);
        reply->mutable_metadata()->set_status("World state stepped forward");

        return Status::OK;
    }

    Status StartSimulation(ServerContext *context, const sim_server::StartSimulationRequest *request,
                           sim_server::SimulationResultResponse *reply) override
    {
        const size_t x_max = request->init_req().dimensions().x_max();
        const size_t y_max = request->init_req().dimensions().y_max();
        const size_t z_max = request->init_req().dimensions().z_max();

        tl::expected<std::tuple<uint64_t, Grid3D>, std::string> world_state = InitWorldStateInternal(x_max, y_max, z_max);
        if (!world_state)
        {
            return Status(grpc::StatusCode::INVALID_ARGUMENT, world_state.error());
        }

        const auto &[id, start_state] = *world_state;

        ConvertGrid3DToProto(start_state, *reply->mutable_start_state()->mutable_state());

        Bitset128 rule;
        std::memcpy(&rule, request->step_req().rule().data(), sizeof(Bitset128));
        const uint64_t num_steps = request->step_req().num_steps();

        const uint64_t timeout = request->has_timeout() ? request->timeout() : kDefaultSimulationTimeoutSeconds;

        Grid3D current_world_state = get_world_state_by_id(id);
        Grid3D end_state;
        auto start_time = std::chrono::steady_clock::now();
        for (uint64_t i = 0; i < num_steps; ++i)
        {
            auto current_time = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= timeout)
            {
                std::cout << "Ending simulation due to timeout" << std::endl;
                break;
            }
            // The temporary object returned by StepWorldStateForwardInternal is moved into end_state
            end_state = StepWorldStateForwardInternal(id, rule); // Move assignment (automatic if move constructor exists)
        }

        // Serialize the updated world state into the response
        sim_server::WorldStateResponse &end_state_proto = *reply->mutable_end_state();
        ConvertGrid3DToProto(end_state, *end_state_proto.mutable_state());
        end_state_proto.mutable_metadata()->set_step(get_step_by_world_state_id(id) + 1);
        end_state_proto.mutable_metadata()->set_status("World state stepped forward");

        reply->set_state_changed_during_sim(!(start_state == end_state));

        // Save the updated world state for future steps
        set_world_state_by_id(id, end_state);

        return Status::OK;
    }

private:
    WorldStateContainer states; // Automatically initialized via WorldStateContainer's default constructor
    std::unordered_map<uint64_t, size_t> world_state_id_to_step;

    Grid3D get_world_state_by_id(const uint64_t world_state_id)
    {
        // Retrieve the current world state from a stored location, e.g., a class member or a database
        // TODO: error handling
        return states.world_states.at(world_state_id);
    }

    // Save the current world state after a step.
    void set_world_state_by_id(const uint64_t world_state_id, const Grid3D &state)
    {
        states.world_states[world_state_id] = state;
    }

    // Keep track of the current step
    size_t get_step_by_world_state_id(const uint64_t world_state_id) const
    {
        // if(world_state_id_to_step.find(world_state_id) != world_state_id_to_step.end()) {
        return world_state_id_to_step.at(world_state_id);
        // }
        // throw std::runtime_error(std::string("Couldn't find mapping for world_state_id: ") + std::to_string(world_state_id));
    }

    void set_step_by_world_state_id(const uint64_t world_state_id, size_t step)
    {
        world_state_id_to_step[world_state_id] = step;
    }

    /**
     * Serializes a Grid3D into the provided Data protobuf message.
     */
    void ConvertGrid3DToProto(const Grid3D &grid, sim_server::Vector3D &vec3d_proto)
    {
        for (const auto &grid2d : grid)
        {
            sim_server::Vector2D *vec2d_proto = vec3d_proto.add_vec2d(); // Add a new Vector2D to the current Vector3D
            for (const auto &grid1d : grid2d)
            {
                sim_server::Vector1D *vec1d_proto = vec2d_proto->add_vec1d(); // Add a new Vector1D to the current Vector2D
                for (const auto &value : grid1d)
                {
                    vec1d_proto->add_bit(static_cast<uint32_t>(value)); // Add 0 or 1 as uint32 to the repeated bit field
                }
            }
        }
    }

    tl::expected<std::tuple<uint64_t, Grid3D>, std::string> InitWorldStateInternal(const size_t x_max, const size_t y_max, const size_t z_max)
    {
        tl::expected<std::tuple<uint64_t, Grid3D>, std::string> state = states.InitWorldState(x_max, y_max, z_max);
        if (!state)
        {
            std::cerr << "Failed to initialize world state!" << std::endl;
            return tl::unexpected(state.error());
        }
        const auto &[id, grid] = *state;
        set_world_state_by_id(id, grid);
        set_step_by_world_state_id(id, 0);
        return state;
    }

    uint32_t hash3DArray(const std::vector<std::vector<std::vector<uint8_t>>> &array)
    {
        uint32_t hash = 0;
        const uint32_t prime = 31;
        for (const auto &arr2D : array)
        {
            for (const auto &arr1D : arr2D)
            {
                for (uint8_t value : arr1D)
                {
                    hash = hash * prime + value;
                }
            }
        }
        return hash;
    }

    Grid3D StepWorldStateForwardInternal(const uint64_t world_state_id, const Bitset128 rule)
    {
        Grid3D current_world_state = get_world_state_by_id(world_state_id);
        Grid3D updated_world_state = states.UpdateWorldState(current_world_state, rule);
        std::cout << "before: " + std::to_string(hash3DArray(current_world_state)) + " after: " + std::to_string(hash3DArray(updated_world_state)) << std::endl;
        set_step_by_world_state_id(world_state_id, get_step_by_world_state_id(world_state_id) + 1);
        return updated_world_state;
    }
};

// Function to start the server
void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    StateServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Enable reflection
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait(); // Keep the server running
};
