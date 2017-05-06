
CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++ grpc`       \
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed \
           -lprotobuf -lpthread -ldl

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`


all: grpc.pb.cc pb.cc

grpc.pb.cc: keyvector.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) keyvector.proto

pb.cc: keyvector.proto
	protoc --cpp_out=. keyvector.proto

paramServer: keyvector.grpc.pb.o  keyvector.pb.o  server.o
	$(CXX) $^ $(LDFLAGS) -o $@

paramClient: keyvector.grpc.pb.o  keyvector.pb.o  client.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o *.pb.cc *.pb.h server client

