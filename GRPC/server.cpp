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

#define NUM_CLIENT 25
//#define MAX_CLIENT 16
#define MAX_KEY 500
#define MAX_VALUE 100
int readyCount;
int pullCount = 0, pushCount = 0;
bool isAggregated, canPush = true, canPull = false;
//double valueTable[MAX_CLIENT][MAX_KEY][MAX_VALUE] = {};
double avgValue[MAX_KEY][MAX_VALUE]= {};
std::mutex pull_mutex, push_mutex;

double * aggregate(int *keys, int numkey) {
	if(isAggregated == false) {
		// do average
		for (int i = 0; i < MAX_KEY; i++)
			for (int j = 0; j < MAX_VALUE; j++)
				avgValue[i][j] /= (double)NUM_CLIENT;
		isAggregated = true;
	}
	return avgValue[0];
}

class ParamServerServiceImpl final : public pushPullRequest::Service {

	Status push(ServerContext* context, const keyVectorMessage* vector, Empty* response) override {

		//Wait for all pull done
		while(!canPush);
		cout << "Pushed. Iteration:" << vector->iter() << endl;


		//Push
		push_mutex.lock();
		for (int i = 0 ; i < MAX_KEY ; ++i) {
			for (int j = 0; j < MAX_VALUE; j++) {
				avgValue[i][j] += vector->val(i*MAX_VALUE + j);
			}
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

		//Wait for all push done.
		while(!canPull);
		cout << "Pulled." << endl;

		for(int i = 0 ; i < numkey ; ++i){
			keys[i] = reqkey->key(i);
		}
		pull_mutex.lock();
		double *avg = aggregate(keys, numkey);

		for(int i = 0; i < MAX_KEY; i++)
			for(int j =0; j < MAX_VALUE; j++)
				retValue->add_val( avg[i*j] );

		if (++pullCount == NUM_CLIENT) {
			//ALL CLIENT IS PULLED
			pullCount = 0;
			canPush = true;
			canPull = false;
			isAggregated = false;
			for(int i = 0; i < MAX_KEY; i++)
				for(int j =0; j < MAX_VALUE; j++)
					avgValue[i][j] = 0;
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
