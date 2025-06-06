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

        auto result = InitWorldStateInternal(x_max, y_max, z_max);
        if (!result)
        {
            return Status(grpc::StatusCode::INVALID_ARGUMENT, result.error());
        }

        const auto &[id, grid] = *result;

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
        Bitset128 rule = ParseBitSetRuleFromString(request->rule());
        const uint64_t world_state_id = request->world_state_id();

        auto step_state_result = StepWorldStateForwardInternal(world_state_id, rule);
        if (!step_state_result)
        {
            return Status(grpc::StatusCode::INTERNAL, step_state_result.error());
        }

        BitPackedGrid3D updated_world_state = *step_state_result;

        // Save the updated state
        set_world_state_by_id(world_state_id, updated_world_state);

        // Serialize the updated world state into the response
        ConvertGrid3DToProto(updated_world_state, *reply->mutable_state());
        reply->mutable_metadata()->set_state_id(world_state_id);

        auto get_step_result = get_step_by_world_state_id(world_state_id);
        if (!get_step_result)
        {
            return Status(grpc::StatusCode::INTERNAL, get_step_result.error());
        }

        size_t new_step = *get_step_result;
        reply->mutable_metadata()->set_step(new_step + 1);
        reply->mutable_metadata()->set_status("World state stepped forward");

        return Status::OK;
    }

    // Only parses rule. Rule is not persisted past lifetime of request
    Status UpdateRule(ServerContext *context, const sim_server::UpdateRuleRequest *request,
                      sim_server::UpdateRuleResponse *reply) override
    {
        reply->set_world_state_id(request->world_state_id());
        reply->set_rule_number(request->rule_number());

        Bitset128 rule = build_from_eca(request->rule_number());

        // Convert to string format
        std::string serialized(reinterpret_cast<const char *>(&rule), sizeof(Bitset128));
        reply->set_rule(serialized);

        return Status::OK;
    }

    Status StartSimulation(ServerContext *context, const sim_server::StartSimulationRequest *request,
                           sim_server::SimulationResultResponse *reply) override
    {
        const size_t x_max = request->init_req().dimensions().x_max();
        const size_t y_max = request->init_req().dimensions().y_max();
        const size_t z_max = request->init_req().dimensions().z_max();

        auto init_state_result = InitWorldStateInternal(x_max, y_max, z_max);
        if (!init_state_result)
        {
            return Status(grpc::StatusCode::INVALID_ARGUMENT, init_state_result.error());
        }

        const auto &[id, start_state] = *init_state_result;

        ConvertGrid3DToProto(start_state, *reply->mutable_start_state()->mutable_state());

        Bitset128 rule = ParseBitSetRuleFromString(request->step_req().rule());
        const uint64_t num_steps = request->step_req().num_steps();
        const uint64_t timeout = request->has_timeout() ? request->timeout() : kDefaultSimulationTimeoutSeconds;

        BitPackedGrid3D end_state(x_max, y_max, z_max);
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
            auto step_state_result = StepWorldStateForwardInternal(id, rule);
            if (!step_state_result)
            {
                return Status(grpc::StatusCode::INTERNAL, step_state_result.error());
            }
            end_state = *step_state_result; // Move assignment (automatic if move constructor exists)
        }

        // Serialize the updated world state into the response
        sim_server::WorldStateResponse &end_state_proto = *reply->mutable_end_state();
        ConvertGrid3DToProto(end_state, *end_state_proto.mutable_state());

        auto step_result = get_step_by_world_state_id(id);
        if (!step_result)
        {
            return Status(grpc::StatusCode::INTERNAL, step_result.error());
        }

        end_state_proto.mutable_metadata()->set_status("World state stepped forward");
        end_state_proto.mutable_metadata()->set_step(*step_result + 1);
        reply->set_state_changed_during_sim(!(start_state == end_state));

        // Save the updated world state for future steps
        set_world_state_by_id(id, end_state);
        return Status::OK;
    }

private:
    WorldStateContainer states; // Automatically initialized via WorldStateContainer's default constructor
    std::unordered_map<uint64_t, size_t> world_state_id_to_step;

    tl::expected<BitPackedGrid3D, std::string> get_world_state_by_id(uint64_t world_state_id) const
    {
        auto it = states.world_states.find(world_state_id);
        if (it == states.world_states.end())
            return tl::unexpected("No world state found for id: " + std::to_string(world_state_id));

        return it->second;
    }

    // Save the current world state after a step.
    void set_world_state_by_id(const uint64_t world_state_id, const BitPackedGrid3D &state)
    {
        states.world_states.insert_or_assign(world_state_id, std::move(state)); // C++17
    }

    // Keep track of the current step
    tl::expected<size_t, std::string> get_step_by_world_state_id(uint64_t world_state_id) const
    {
        auto it = world_state_id_to_step.find(world_state_id);
        if (it != world_state_id_to_step.end())
            return it->second;

        return tl::unexpected("Couldn't find mapping for world_state_id: " + std::to_string(world_state_id));
    }

    void set_step_by_world_state_id(const uint64_t world_state_id, size_t step)
    {
        world_state_id_to_step[world_state_id] = step;
    }

    /**
     * Serializes a BitPackedGrid3D into the provided Data protobuf message.
     */
    void ConvertGrid3DToProto(const BitPackedGrid3D &grid, sim_server::Vector3D &vec3d_proto)
    {
        const size_t x_max = grid.x_max;
        const size_t y_max = grid.y_max;
        const size_t z_max = grid.z_max;

        for (size_t x = 0; x < x_max; ++x)
        {
            sim_server::Vector2D *vec2d_proto = vec3d_proto.add_vec2d();
            for (size_t y = 0; y < y_max; ++y)
            {
                sim_server::Vector1D *vec1d_proto = vec2d_proto->add_vec1d();
                for (size_t z = 0; z < z_max; ++z)
                {
                    bool bit = grid.get(x, y, z);
                    vec1d_proto->add_bit(static_cast<uint32_t>(bit));
                }
            }
        }
    }

    tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> InitWorldStateInternal(const size_t x_max, const size_t y_max, const size_t z_max)
    {
        tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> state = states.InitWorldState1D(x_max, y_max, z_max);
        if (!state)
        {
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

    tl::expected<BitPackedGrid3D, std::string> StepWorldStateForwardInternal(uint64_t world_state_id, Bitset128 rule)
    {
        return get_world_state_by_id(world_state_id)
            .transform([&](const BitPackedGrid3D &current)
                       {
            BitPackedGrid3D updated = states.UpdateWorldState(current, rule);
            return updated; })
            .and_then([&](BitPackedGrid3D updated) -> tl::expected<BitPackedGrid3D, std::string>
                      { return get_step_by_world_state_id(world_state_id)
                            .transform([&](size_t step)
                                       {
                                           set_step_by_world_state_id(world_state_id, step + 1);
                                           return updated; // propagate updated world state
                                       }); });
    }
};

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

    server->Wait();
};
