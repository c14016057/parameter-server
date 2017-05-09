#include <iostream>
#include <pthread.h>
#include <time.h>
#include <memory>
#include <grpc++/grpc++.h>
#include "keyvector.grpc.pb.h"

#define NUM_KEY 500
#define NUM_VALUE 100
#define THREAD_NUM 1000

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using keyvector::pushPullRequest;
using keyvector::Empty;
using keyvector::keyMessage;
using keyvector::vectorMessage;
using keyvector::keyVectorMessage;

using std::cout;
using std::endl;

class paramClient {
	public:
		paramClient(std::shared_ptr<Channel> channel) : stub_(pushPullRequest::NewStub(channel)) {}


		void pull(int* key, int numKey) {
			ClientContext context;
			keyMessage keyMsg;
			vectorMessage resValue;

			for(int i = 0 ; i < numKey ; ++i)
				keyMsg.add_key( key[i] );



			Status status = stub_->pull(&context, keyMsg, &resValue);	
			if (status.ok()) { 
				for (int i = 0; i < NUM_KEY; i++)
					for (int j = 0; j <NUM_VALUE; j++)
						if (resValue.val(i*NUM_VALUE+j) !=1 ) {
							std::cout << "error avg" << std::endl;
							return;
						}
				cout << "ac avg" << std::endl;
			}
			else{ 
				std::cout << status.error_code() << ": " << status.error_message() << std::endl;
				return ;
			}

			//		cout << response.val(0) << endl;
		}


		void push(int* keys, double** vals, int numKey, int numVal, int iter) {
			ClientContext context;
			keyVectorMessage kvMsg;
			Empty response;


			for(int i = 0 ; i < numKey ; ++i) {
				kvMsg.add_key( keys[i] );
				for(int j = 0 ; j < numVal ; ++j)
					kvMsg.add_val( vals[i][j] );
			}
			kvMsg.set_iter(iter);


			Status status = stub_->push(&context, kvMsg, &response);
			if (status.ok()) 
				cout << "ok, Pushed." << endl;
			else
				cout << status.error_code() << ": " << status.error_message() << endl;

		}

	private:
		std::unique_ptr<pushPullRequest::Stub> stub_;
};


double dRandGen(double dMin, double dMax) {
	double dRand = (double)rand() / RAND_MAX;
	return dMin + dRand * (dMax - dMin);

}

int main(int argc, char** argv) {
	//paramClient client(grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials()));

	grpc::ChannelArguments channelArgs;
    channelArgs.SetInt("GRPC_ARG_MAX_MESSAGE_LENGTH", 100 * 1024 * 1024);
	paramClient client(grpc::CreateCustomChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials(), channelArgs));
	int* keys = new int[NUM_KEY];
	for(int i = 0 ; i < NUM_KEY ; ++i)
		keys[i] = i;

	double** vals = new double *[NUM_KEY];
	for(int i = 0 ; i < NUM_KEY ; ++i)
		vals[i] = new double[NUM_VALUE];


	for(int i = 0 ; i < 10000 ; ++i) {      // ML Iteration.

		for(int j = 0 ; j < NUM_KEY ; ++j)
			for(int k = 0 ; k < NUM_VALUE ; ++k)
				vals[j][k] = 1;

		client.push(keys, (double **)vals, NUM_KEY, NUM_VALUE, i);


		//	DO ML COMPUTE.


		client.pull(keys, NUM_KEY);
	}


	for(int i = 0 ; i < NUM_KEY ; ++i)
		delete [] vals[i];
	delete [] vals;
	delete [] keys;


	return 0;
}

