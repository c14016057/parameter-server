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


int readyCount;
bool isAggregated;
std::mutex mutex_;

double aggregate(int *keys) {
	// Do Sync.

	while(readyCount != 2)
		;

	return 0.87;		
}


class ParamServerServiceImpl final : public pushPullRequest::Service {

  Status push(ServerContext* context, const keyVectorMessage* request, Empty* response) override {

	
	cout << "Pushed. Iteration:" << request->iter() << endl;

	/*
	for(int i = 1 ; i < 100 ; ++i){
		cout << request->key(i) << ":" << request->val(i) << endl;
	}*/

	mutex_.lock();
	++readyCount;
	mutex_.unlock();

	return Status::OK;
  }
  
  Status pull(ServerContext* context, const keyMessage* request, vectorMessage* response) override {
	
	int numkey = request->key_size();
	int *keys = new int[numkey];

	cout << "Pulled." << endl;
	

	for(int i = 0 ; i < numkey ; ++i){
		keys[i] = request->key(i);
	}

//	mutex_.lock();
	double avg = aggregate(keys);
//	mutex_.unlock();

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
	isAggregated = false;
	RunServer();

	return 0;
}




