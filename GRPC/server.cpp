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
//#define MAX_CLIENT 16
#define MAX_KEY 5000
#define MAX_VALUE 1000
#define MAXP MAX_KEY*MAX_VALUE
#define SENDTYPE double
int readyCount;
int pullCount = 0, pushCount = 0;
bool isAggregated, canPush = true, canPull = false;
SENDTYPE avgValue[MAX_KEY*MAX_VALUE]= {};
std::mutex pull_mutex, push_mutex;

SENDTYPE * aggregate(int *keys, int numkey, int numP) {
	if(isAggregated == false) {
		// do average
		for (int i = 0; i < numP; i++)
				avgValue[i] /= (double)NUM_CLIENT;
		isAggregated = true;
	}
	return avgValue;
}

class ParamServerServiceImpl final : public pushPullRequest::Service {

	Status push(ServerContext* context, const keyVectorMessage* vector, Empty* response) override {

		//Wait for all pull done
		while(!canPush);
		cout << "Pushed. Iteration:" << vector->iter() << endl;


		//Push
		push_mutex.lock();


		int numP = vector->nump();
		for (int i = 0; i < numP; i++) {
			avgValue[i] += vector->val(i);
		}
		if (++pushCount == NUM_CLIENT) {
			//ALL CLIENT IS PUSHED
			pushCount = 0;
			canPull = true;
			canPush = false;
		}
		push_mutex.unlock();



		return Status::OK;
	}

	Status pull(ServerContext* context, const keyMessage* reqkey, vectorMessage* retValue) override {

		int numkey = reqkey->key_size();
		int *keys = new int[numkey];
		int numP = reqkey->nump();
		//Wait for all push done.
		while(!canPull);
		cout << "Pulled." << endl;
		/*
		for(int i = 0 ; i < numkey ; ++i){
			keys[i] = reqkey->key(i);
		}*/
		pull_mutex.lock();
		SENDTYPE *avg = aggregate(keys, numkey, numP);

		for(int i = 0; i < numP; i++)
				retValue->add_val( avg[i] );

		if (++pullCount == NUM_CLIENT) {
			//ALL CLIENT IS PULLED
			pullCount = 0;
			canPush = true;
			canPull = false;
			isAggregated = false;
			for(int i = 0; i < MAXP; i++)
					avgValue[i] = 0;
		}
		pull_mutex.unlock();
				


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
