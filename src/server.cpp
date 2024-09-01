#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "grpcdemo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpcdemo::DataService;
using grpcdemo::RequestMessage;
using grpcdemo::ResponseMessage;
using grpcdemo::Metadata;
using grpcdemo::Data;
using grpcdemo::Vector3D;
using grpcdemo::Vector2D;

class DataServiceImpl final : public DataService::Service {
public:
    Status GetData(ServerContext* context, const RequestMessage* request,
                   ResponseMessage* reply) override {
        // Log received metadata for debugging
        std::cout << "Received request with step: " << request->metadata().step() 
                  << " and status: " << request->metadata().status() << std::endl;

        // Fill the response metadata
        Metadata* response_metadata = reply->mutable_metadata();
        response_metadata->set_step(request->metadata().step());
        response_metadata->set_status("Processed " + request->metadata().status());

        // Process data and fill the response data
        Data* response_data = reply->mutable_data();
        for (const auto& vec3d : request->data().data()) {
            Vector3D* response_vec3d = response_data->add_data();
            for (const auto& vec2d : vec3d.vec2d()) {
                Vector2D* response_vec2d = response_vec3d->add_vec2d();
                for (const auto& vec1d : vec2d.vec1d()) {
                    std::string processed_bytes = processVector(vec1d); // Dummy processing function
                    response_vec2d->add_vec1d(processed_bytes);
                }
            }
        }

        return Status::OK;
    }

private:
    // This function is a placeholder for any processing you need on the vector data.
    std::string processVector(const std::string& input) {
        // Just returning the input in this example. Replace with actual logic.
        return input;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    DataServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();

    return 0;
}