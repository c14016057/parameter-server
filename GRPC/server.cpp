#include <iostream>
#include <memory>
#include <string>

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

double aggregate(int *keys) {
	// Do Sync.

}


class ParamServerServiceImpl final : public pushPullRequest::Service {

  Status push(ServerContext* context, const keyVectorMessage* request, Empty* response) override {

	/*
	std::cout << "Pushed. key:" << request->key() << std::endl;
//	std::cout << "Size: " <<  request->v_size() << std::endl;
	
	for(int i = 1 ; i < request->v_size() ; ++i)
		std::cout << request->v(i) << std::endl;
	*/



	cout << "Pushed. Iteration:" << request->iter() << endl;



	
	return Status::OK;
  }
  
  Status pull(ServerContext* context, const keyMessage* request, vectorMessage* response) override {
	
	int numkey = request->key_size();
	int *keys = new int[numkey];

	cout << "Pulled." << endl;
	

	for(int i = 0 ; i < numkey ; ++i){
		keys[i] = request->key(i);
	}

	aggregate(keys);
	
	
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
	RunServer();

	return 0;
}




