#include <iostream>
#include <pthread.h>
#include <time.h>
#include <memory>
#include <grpc++/grpc++.h>
#include "keyvector.grpc.pb.h"

#define NUM_KEY 5000
#define NUM_VALUE_PERKEY 1000

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


	void pull(int* key, int numKey){
		ClientContext context;
		keyMessage keyMsg;
		vectorMessage response;

		for(int i = 0 ; i < numKey ; ++i)
			keyMsg.add_key( key[i] );
		
		

		Status status = stub_->pull(&context, keyMsg, &response);	
		if (status.ok()) 
			std::cout << "ok, Pulled. avg = " << response.val(0) << std::endl;
		else{ 
			std::cout << status.error_code() << ": " << status.error_message() << std::endl;
			return ;
		}

//		cout << response.val(0) << endl;

		
	}


	void push(int* keys, double** vals, int numKey, int numVal, int iter){
		ClientContext context;
                keyVectorMessage kvMsg;
                Empty response;

		
		for(int i = 0 ; i < numKey ; ++i){
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


double dRandGen(double dMin, double dMax){
	double dRand = (double)rand() / RAND_MAX;
	return dMin + dRand * (dMax - dMin);

}

int main(int argc, char** argv) {
	paramClient client(grpc::CreateChannel("140.112.30.241:50051", grpc::InsecureChannelCredentials()));


	int* keys = new int[NUM_KEY];
	for(int i = 0 ; i < NUM_KEY ; ++i)
                keys[i] = i+1;

	double** vals = new double *[NUM_KEY];
	for(int i = 0 ; i < NUM_KEY ; ++i)
	    vals[i] = new double[NUM_VALUE_PERKEY];

	
	for(int i = 0 ; i < 10 ; ++i){      // ML Iteration.

		for(int j = 0 ; j < NUM_KEY ; ++j)
			for(int k = 0 ; k < NUM_VALUE_PERKEY ; ++k)
				vals[j][k] = dRandGen(0, 1);
	
		client.push(keys, (double **)vals, NUM_KEY, NUM_VALUE_PERKEY, i);
	
		
		//	DO ML COMPUTE.
		

		client.pull(keys, NUM_KEY);
	}
	
	
	for(int i = 0 ; i < NUM_KEY ; ++i)
            delete [] vals[i];
	delete [] vals;
	delete [] keys;
	

	return 0;
}

