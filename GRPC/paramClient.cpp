#include <iostream>
#include <pthread.h>
#include <time.h>
#include <memory>

#include <grpc++/grpc++.h>

#include "keyvector.grpc.pb.h"

#define THREAD_NUM 1000

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using keyvector::pushPullRequest;
using keyvector::Empty;
using keyvector::keyMessage;
using keyvector::vectorMessage;
using keyvector::keyVectorMessage;


class paramClient {
	public:
		paramClient(std::shared_ptr<Channel> channel) : stub_(pushPullRequest::NewStub(channel)) {}

	void pull(const int key){

		ClientContext context;
		keyMessage keyMsg;
		vectorMessage response;

		keyMsg.set_key(key);
		Status status = stub_->pull(&context, keyMsg, &response);

		if (status.ok()) {
			std::cout << "ok, Pulled." << std::endl;
			for(int i = 0 ; i < response.v_size() ; ++i)
				 std::cout << response.v(i) << std::endl;
		}
		else {
			std::cout << status.error_code() << ": " << status.error_message() << std::endl;
		}
	}

	void push(){
		ClientContext context;
                keyVectorMessage kvMsg;
                Empty response;

                kvMsg.set_key(234);

		for(int i = 1 ; i < 10 ; ++i)
			kvMsg.add_v(double(1)/1234);
                Status status = stub_->push(&context, kvMsg, &response);

                if (status.ok()) {
                        std::cout << "ok, Pushed." << std::endl;
                }
                else {
                        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                }
		
	}
	
	private:
		std::unique_ptr<pushPullRequest::Stub> stub_;
};


int main(int argc, char** argv) {
	paramClient client(grpc::CreateChannel("140.112.30.241:50051", grpc::InsecureChannelCredentials()));
	
	int key = 123;
	client.pull(key);
	client.push();
	return 0;
}

