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
#define FITNESS_COLLECTION "fitness"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using api::InitRequest;
using api::FitnessResponse;
using api::FitnessRequest;
using api::StopRequest;
using api::StopResponse;

/*
 * Implement Protobuf OperationalResearch Service
 */
class ORServiceImpl final : public api::OperationalResearch::Service {

private:
    mongocxx::client conn;
    mongocxx::collection transac_coll;
    mongocxx::collection fitness_coll;

public:

    /*
     * Constructor
     */
    ORServiceImpl(){
        mongocxx::instance inst{};
        conn = mongocxx::uri{};
        mongocxx::database db = conn[BDD];
        transac_coll = db[TRANSACTION_COLLECTION];
        fitness_coll = db[FITNESS_COLLECTION];
    }

    /*
     * Implement ProtoBuff Rpc Methods
     */
    Status InitConversation(ServerContext* context, const InitRequest* request,
                    FitnessResponse* reply) override {

        std::string _id(generateId());

        // set the response
        reply->set_id(_id);
        reply->set_solution(getNeighbourSolution(request->solution()));

        // save data in mongodb
        bsoncxx::builder::stream::document documentTransaction{};
        documentTransaction << "transaction_id" << _id;
        documentTransaction << "customer" << request->customer();
        documentTransaction << "solution_initial" << request->solution();
        documentTransaction << "solution_size" << request->solutionsize();


        bsoncxx::builder::stream::document documentFitness{};
        documentFitness << "transaction_id" << _id;
        documentFitness << "solution" << request->solution();
        documentFitness << "fitness" << request->fitness();
        bsoncxx::types::value  fitnessId = fitness_coll.insert_one(documentFitness.view())->inserted_id();

        documentTransaction << "best_fitness_id" << fitnessId;
        documentTransaction << "algorithm" << request->algorithm();
        transac_coll.insert_one(documentTransaction.view());

        return Status::OK;
    }


    Status SendFitness(ServerContext* context, const FitnessRequest* request,
                            FitnessResponse* reply) override {

        //set the response
        reply->set_id(request->id());
        reply->set_solution(getNeighbourSolution(request->solution()));

        // save data in mongodb
        bsoncxx::builder::stream::document document{};
        document << "transaction_id" << request->id();
        document << "solution" << request->solution();
        document << "fitness" << request->fitness();

        fitness_coll.insert_one(document.view());

        return Status::OK;
    }


};

/*
 * Run Server method
 */
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
