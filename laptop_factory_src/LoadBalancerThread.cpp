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
    int request_type;
    CustomerRecord record;

    while (true) {
        CustomerRequest request = stub.ReceiveCustomerRequest();
        if (!request.IsValid()) {
            break;
        }
        request_type = request.GetRequestType();
        LaptopInfo info;
        CustomerRecord record;
        ServerClientInterface inter;
        switch (request_type) {

            case 1:
                info = SendToServer(request, INFO);
                inter.info = info;
                stub.Ship(inter, INFO);
                break;

            case 2:
                record = SendToServer(request, RECORD);
                inter.record = record;
                stub.Ship(inter, RECORD);
                break;
            default:
                std::cout << "Undefined laptop type: "
                          << request_type << std::endl;
        }
    }
}

void LoadBalancerWorker::ConnectServers(std::vector<ServerInfo> vector1) {

    for(auto v: vector1){
        ServersStubsMap[v.unique_id] = std::move(new MultiPurposeClientSocket);
        ServersStubsMap[v.unique_id]->Init( v.peer_ip,v.port_no);
        SendIdentification();
    }
}

void LoadBalancerWorker::SendIdentification() {
    char buffer[32];
    HandShaking id(0, 0);
    int size = id.Size();

    id.Marshal(buffer);
    ServersStubsMap[0]->Send(buffer, size, 0);
}

ServerClientInterface LoadBalancerWorker::SendToServer(CustomerRequest request, ServerClientInterfaceOp operation) {
    ServerClientInterface result;
    char buffer[32];
    int size;
    request.Marshal(buffer);
    size = request.Size();
    if (ServersStubsMap[0]->Send(buffer, size, 0)) {
        if(operation == INFO){
            LaptopInfo info;
            size = info.Size();
            if (ServersStubsMap[0]->Recv(buffer, size, 0)) {
                info.Unmarshal(buffer);
                result.info = info;

            }
        }else if(operation == RECORD){
                CustomerRecord record;
                size = record.Size();
                if (ServersStubsMap[0]->Recv(buffer, size, 0)) {
                    record.Unmarshal(buffer);
                    result.record = record;

                }
        }

    }

    return result;
}

ServerInfo::ServerInfo(int id, int port, std::string ip) {
    unique_id = id;
    port_no = port;
    peer_ip = ip;
}
