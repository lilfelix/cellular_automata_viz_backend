#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "sim_server.grpc.pb.h"
#include "world_state.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using sim_server::Data;
using sim_server::DataService;
using sim_server::Metadata;
using sim_server::RequestMessage;
using sim_server::ResponseMessage;
using sim_server::Vector2D;
using sim_server::Vector3D;

class DataServiceImpl final : public DataService::Service
{
public:
    Status GetData(ServerContext *context, const RequestMessage *request,
                   ResponseMessage *reply) override
    {
        // Log received metadata for debugging
        std::cout << "Received request with step: " << request->metadata().step()
                  << " and status: " << request->metadata().status() << std::endl;

        // Fill the response metadata
        Metadata *response_metadata = reply->mutable_metadata();
        response_metadata->set_step(request->metadata().step());
        response_metadata->set_status("Processed " + request->metadata().status());

        // Assume we are generating or receiving the Grid3D data to be serialized
        size_t x_max = 10; // Example size, you can get this dynamically or from the request
        size_t y_max = 10;
        size_t z_max = 10;

        // Generate the Grid3D data (or use the request data)
        std::tuple<uint64_t, Grid3D> state = states.InitWorldState(x_max, y_max, z_max);

        // Serialize the Grid3D data into the response protobuf
        Data *response_data = reply->mutable_data();
        ConvertGrid3DToProto(std::get<1>(state), response_data);

        return Status::OK;
    }

    Status InitWorldState(ServerContext *context, const sim_server::InitializeRequest *request,
                                sim_server::WorldStateResponse *reply) override
    {
        size_t x_max = request->dimensions().x_max();
        size_t y_max = request->dimensions().y_max();
        size_t z_max = request->dimensions().z_max();

        // Generate the initial world state
        std::tuple<uint64_t, Grid3D> state = states.InitWorldState(x_max, y_max, z_max);

        // Serialize the generated world state into the response
        ConvertGrid3DToProto(std::get<1>(state), reply->mutable_data());

        // Set up the metadata
        sim_server::Metadata *metadata = reply->mutable_metadata();
        metadata->set_step(0);
        metadata->set_status("World state initialized");

        return Status::OK;
    }

    Status StepWorldStateForward(ServerContext *context, const sim_server::StepRequest *request,
                                 sim_server::WorldStateResponse *reply) override
    {
        uint64_t world_state_id = request->world_state_id();

        // Deserialize the rule
        Bitset128 rule;
        std::memcpy(&rule, request->rule().data(), sizeof(Bitset128));

        // Get the current world state
        Grid3D current_world_state = get_current_world_state(request->world_state_id());

        // Step the world state forward
        Grid3D updated_world_state = states.UpdateWorldState(current_world_state, rule);

        // Serialize the updated world state into the response
        ConvertGrid3DToProto(updated_world_state, reply->mutable_data());

        // Set up the metadata
        sim_server::Metadata *metadata = reply->mutable_metadata();
        metadata->set_step(get_current_step() + 1); // Increment step count
        metadata->set_status("World state stepped forward");

        // Optionally save the updated world state for future steps
        save_current_world_state(request->world_state_id(), updated_world_state);

        return Status::OK;
    }

private:
    
    WorldStateContainer states; // Automatically initialized via WorldStateContainer's default constructor
    // This is a placeholder; you need to implement this to get the current world state.
    Grid3D get_current_world_state(const uint64_t world_state_id)
    {
        // Retrieve the current world state from a stored location, e.g., a class member or a database
        // TODO: error handling
        return states.world_states.at(world_state_id);
    }

    // Placeholder for saving the current world state after a step.
    void save_current_world_state(const uint64_t world_state_id, const Grid3D &state)
    {
        // Store the state for future steps
        states.world_states.insert({world_state_id, state});
    }

    // Placeholder to keep track of the current step
    size_t get_current_step() const
    {
        return current_step_;
    }

    void set_current_step(size_t step)
    {
        current_step_ = step;
    }

    size_t current_step_ = 0; // Start at step 0

    /**
     * Serializes a Grid3D into the provided Data protobuf message.
     */
    void ConvertGrid3DToProto(const Grid3D &grid, sim_server::Data *data_proto)
    {
        for (const auto &grid2d : grid)
        {
            sim_server::Vector3D *vec3d_proto = data_proto->add_data(); // Add a new Vector3D to the Data message
            for (const auto &grid1d : grid2d)
            {
                sim_server::Vector2D *vec2d_proto = vec3d_proto->add_vec2d(); // Add a new Vector2D to the current Vector3D
                for (const auto &value : grid1d)
                {
                    vec2d_proto->add_vec1d(static_cast<int32_t>(value)); // Add 0 or 1 as integers
                }
            }
        }
    }
};

// Function to start the server
void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    DataServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Enable reflection
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait(); // Keep the server running
};
