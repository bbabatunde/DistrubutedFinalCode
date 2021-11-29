//
// Created by Egbantan Babatunde on 11/14/21.
//


#include "LoadBalancerThread.h"
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
        CustomerRecord cacheToSend;
        int hashed_server;
        int customer_id = 0;
        std::string cache_string;
        switch (request_type) {
            case 1:
                customer_id = request.GetCustomerId();
                hashed_server = ConsistentHashingAlgorithm(customer_id);
                //@TODO Distribution algorithm Osman
                //@TODO read map of servers to customer id from client(4) to check that algorithm works
                info = SendToServer(request, INFO, hashed_server);
                inter.info = info;
                stub.Ship(inter, INFO);
                
                // std::cout << "size of cache is: " << cache.getMaxSize() << std::endl;
                // invalidating after successful write
                cache.removeRecord(customer_id); // removeRecord is returning bool
                break;

            
            case 2:
                customer_id = request.GetCustomerId();
                hashed_server = ConsistentHashingAlgorithm(customer_id);

                // first check cache and we have a hit nice then return it
                if(cache.hasKey(customer_id)) {
                    record = CustomerRecord(customer_id, cache.getRecord(customer_id));
                } else {
                    record = SendToServer(request, RECORD, hashed_server);
                }
                
                inter.record = record;
                stub.Ship(inter, RECORD);
                // add it to the cache
                // add only valid orders since we add it to cache after first read some -1 were in the list and I tried to prevent it with an if
                if(record.last_order != -1){
                    cache.addRecord(customer_id, record.last_order);
                }
                break;
            case 5:
                cache_string = ""; 
                cache_string = cache.toString();
                if (!cache.empty()){
                    stub.ShipCacheToClient(cache_string);
                } 
                else {
                    stub.ShipCacheToClient("Cache is empty");
                } 
                
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

ServerClientInterface
LoadBalancerWorker::SendToServer(CustomerRequest request, ServerClientInterfaceOp operation, int server) {
    ServerClientInterface result;
    char buffer[32];
    int size;
    request.Marshal(buffer);
    size = request.Size();
    if (ServersStubsMap[server]->Send(buffer, size, 0)) {
        if(operation == INFO){
            LaptopInfo info;
            size = info.Size();
            if (ServersStubsMap[server]->Recv(buffer, size, 0)) {
                info.Unmarshal(buffer);
                result.info = info;

            }
        }else if(operation == RECORD){
                CustomerRecord record;
                size = record.Size();
                if (ServersStubsMap[server]->Recv(buffer, size, 0)) {
                    record.Unmarshal(buffer);
                    result.record = record;

                }
        }

    }

    return result;
}

int LoadBalancerWorker::ConsistentHashingAlgorithm(int id) {
    return 0;
}

ServerInfo::ServerInfo(int id, int port, std::string ip) {
    unique_id = id;
    port_no = port;
    peer_ip = ip;
}

void LoadBalancerWorker::SetCacheSize(int size) {
    cache.setCacheSize(size);
}
