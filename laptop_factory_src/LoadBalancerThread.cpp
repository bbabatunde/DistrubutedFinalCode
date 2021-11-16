//
// Created by Egbantan Babatunde on 11/14/21.
//

#include "LoadBalancerThread.h"
#include "LoadBalancerStub.h"
#include <iostream>

LoadBalancerWorker::LoadBalancerWorker() {

}

void LoadBalancerWorker::BalancerThread(std::unique_ptr<MultiPurposeServerSocket> socket) {
    LoadBalancerStub stub;
    stub.Init(std::move(socket));


    std::cout<<"connections to client"<<std::endl;
    while (true) {
        char* request = stub.ReadClientBuffer();
        if (!request) {
            continue;
        }
        std::cout<<request<<std::endl;
        ForwardCustomerRequest(request);
    }
}

void LoadBalancerWorker::ConnectServers(std::vector<ServerInfo> vector1) {

    for(auto v: vector1){
        ServersStubsMap[v.unique_id] = std::move(new MultiPurposeClientSocket);
        ServersStubsMap[v.unique_id]->Init( v.peer_ip,v.port_no);
        SendIdentification();
    }
}

void LoadBalancerWorker::ForwardCustomerRequest(char *request) {
    ServersStubsMap[0]->Send(request, 100, 0);

}

void LoadBalancerWorker::SendIdentification() {
    char buffer[32];
    HandShaking id(0, 0);
    int size = id.Size();

    id.Marshal(buffer);
    ServersStubsMap[0]->Send(buffer, size, 0);
}

ServerInfo::ServerInfo(int id, int port, std::string ip) {
    unique_id = id;
    port_no = port;
    peer_ip = ip;
}
