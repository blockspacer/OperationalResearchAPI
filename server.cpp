#include <iostream>
#include <memory>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <grpc++/grpc++.h>
#include "libs/helper.h"

#ifdef BAZEL_BUILD
#include "protos/api.proto"
#else
#include "protoClassServer/api.grpc.pb.h"
#endif

#define BDD "api"
#define TRANSACTION_COLLECTION "transactions"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using api::InitRequest;
using api::FitnessResponse;
using api::FitnessRequest;
using api::StopRequest;
using api::StopResponse;


class ORServiceImpl final : public api::OperationalResearch::Service {

private:
    mongocxx::client conn;
    mongocxx::collection transac_coll;

public:

    ORServiceImpl(){
        mongocxx::instance inst{};
        conn = mongocxx::uri{};
        transac_coll = conn[BDD][TRANSACTION_COLLECTION];
    }

    Status InitConversation(ServerContext* context, const InitRequest* request,
                    FitnessResponse* reply) override {

        std::string _id(generateId());

        // set the response
        reply->set_id(_id);
        reply->set_solution(getNeighbourSolution(request->solution()));

        // save data in mongodb
        bsoncxx::builder::stream::document document{};
        document << "transaction_id" << _id;
        document << "customer" << request->customer();
        document << "solution_initial" << request->solution();
        document << "solution_size" << request->solutionsize();
        document << "number_of_evaluation" << request->evalnb();
        document << "algorithm" << request->algorithm();
        transac_coll.insert_one(document.view());

        return Status::OK;
    }


};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    ORServiceImpl service;

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