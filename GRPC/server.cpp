#include <iostream>
#include <memory>
#include <string>
#include <mutex>

#include <grpc++/grpc++.h>
#include "keyvector.grpc.pb.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using keyvector::pushPullRequest;
using keyvector::Empty;
using keyvector::keyMessage;
using keyvector::vectorMessage;
using keyvector::keyVectorMessage;

using std::cout;
using std::endl;

#define NUM_CLIENT 2

int readyCount;
int pullCount;
bool isAggregated;
double avg;
std::mutex mutex_;

double aggregate(int *keys) {
        // Do Sync.

        while(pullCount != 0)
                ;

        mutex_.lock();
        ++readyCount;
        mutex_.unlock();

        while(readyCount != NUM_CLIENT)
                ;

        mutex_.lock();
        if(isAggregated == false) {
                // do average

                avg = 0.87;
                isAggregated = true;
                ++pullCount;
                mutex_.unlock();
        }
        else {
                // read average
                avg = 0.78;

                if( ++pullCount == NUM_CLIENT ) {
                        isAggregated = false;
                        pullCount = 0;
                        readyCount = 0;
                }
                mutex_.unlock();
        }

        return avg;
}

class ParamServerServiceImpl final : public pushPullRequest::Service {

  Status push(ServerContext* context, const keyVectorMessage* request, Empty* response) override {


        cout << "Pushed. Iteration:" << request->iter() << endl;

        /*
        for(int i = 1 ; i < 100 ; ++i){
                cout << request->key(i) << ":" << request->val(i) << endl;
        }*/


        return Status::OK;
  }

  Status pull(ServerContext* context, const keyMessage* request, vectorMessage* response) override {

        int numkey = request->key_size();
        int *keys = new int[numkey];

        cout << "Pulled." << endl;


        for(int i = 0 ; i < numkey ; ++i){
                keys[i] = request->key(i);
        }

        double avg = aggregate(keys);

        response->add_val(avg);



        delete [] keys;
        return Status::OK;
  }
};

void RunServer() {
        std::string server_address("0.0.0.0:50051");
        ParamServerServiceImpl service;

        ServerBuilder builder;
        builder.SetMaxMessageSize(100 * 1024 * 1024);
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;

        server->Wait();
}

int main(int argc, char** argv) {
        readyCount = 0;
        pullCount = 0;
        isAggregated = false;
        RunServer();

        return 0;
}
