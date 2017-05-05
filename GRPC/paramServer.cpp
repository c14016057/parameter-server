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

class ParamServerServiceImpl final : public pushPullRequest::Service {
  Status push(ServerContext* context, const keyVectorMessage* request, Empty* response) override {
	std::cout << "Pushed. key:" << request->key() << std::endl;
//	std::cout << "Size: " <<  request->v_size() << std::endl;
	
	for(int i = 1 ; i < request->v_size() ; ++i)
		std::cout << request->v(i) << std::endl;
	
	return Status::OK;
  }
  
  Status pull(ServerContext* context, const keyMessage* request, vectorMessage* response) override {
	std::cout << "Pulled. Key:"<< request->key() << std::endl;

	for(int i = 1 ; i < 10 ; ++i)
	       response->add_v( i / double(100) );
	return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  ParamServerServiceImpl service;

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




