//
// Created by Egbantan Babatunde on 11/14/21.
//

#include "LoadBalancerThread.h"

#include <utility>

LoadBalancerWorker::LoadBalancerWorker(){

};

void LoadBalancerWorker::BalancerThread(std::unique_ptr<MultiPurposeServerSocket> socket) {



    LoadBalancerStub stub;
    stub.Init(std::move(socket));

    HandShaking role = stub.RecieveIdentification();
    if(role.message == 0){
        CustomerThread(std::move(stub));
    }else if(role.message == 1){
        SysAdminThread(std::move(stub));
    }else{
        std::cout<<"Error: invalid role"<<std::endl;

    }

}

void LoadBalancerWorker::InitRing(std::vector<ServerInfo> vector1, LoadBalancerRing ring1) {
    old_ring = ring;
    ring = std::move(ring1);

    for(auto v: vector1){
        ServersStubsMap[v.unique_id] = std::move(new MultiPurposeClientSocket);
        ServersStubsMap[v.unique_id]->Init( v.peer_ip,v.port_no);
        SendIdentification(v.unique_id);
    }

    Migrate(old_ring, ring);
}

void LoadBalancerWorker::SendIdentification(int sid) {
    char buffer[32];
    HandShaking id(0, 0);
    int size = id.Size();

    id.Marshal(buffer);
    ServersStubsMap[sid]->Send(buffer, size, 0);
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

void LoadBalancerWorker::CustomerThread(LoadBalancerStub &&stub) {

    int request_type;

    while (true) {
        CustomerRequest request = stub.ReceiveCustomerRequest();
        if (!request.IsValid()) {
            break;
        }
        request_type = request.GetRequestType();
        LaptopInfo info;
        CustomerRecord record;
        ServerClientInterface inter;
        int hashed_server = 0;
        int customer_id = 0;
        int random = 0;
        std::vector<int> nodes;

        switch (request_type) {

            case 1:
                customer_id = request.GetCustomerId();
                //@TODO read map of servers to customer id from client(4) to check that algorithm works
                nodes =  ring.GetNodes(customer_id);

                for(auto node: nodes){
                    info = SendToServer(request, INFO, node);
                }

                inter.info = info;
                stub.Ship(inter, INFO);
                break;

            case 2:

                customer_id = request.GetCustomerId();
                nodes =  ring.GetNodes(customer_id);
                random = rand() % (nodes.size());

                hashed_server = nodes[random];
                record = SendToServer(request, RECORD, hashed_server);

                inter.record = record;
                stub.Ship(inter, RECORD);
                break;
            default:
                std::cout << "Undefined request_type type: "
                          << request_type << std::endl;


        }
    }
}

void LoadBalancerWorker::SysAdminThread(LoadBalancerStub &&stub) {
    int request_type;
    std::vector<ServerInfo> servers;
    size_t servers_count;

    while (true) {
        AdminRequest request = stub.ReceiveAdminRequest();

        request_type = request.GetRequestType();


        switch (request_type) {

            case 1:
                servers.push_back(request.GetServerInfo());
                break;
            default:
                servers_count = servers.size();

                LoadBalancerRing new_ring = LoadBalancerRing(ring.GetServersCount() + servers_count, ring.GetNumberofReplicas());


                InitRing(servers, new_ring);
                return;


        }
    }
}

void LoadBalancerWorker::Migrate(LoadBalancerRing old_ring, LoadBalancerRing new_ring) {

    for(int i = 0; i < old_ring.GetDataCount(); i++){

        std::vector<int> old_nodes = old_ring.GetNodes(i);
        std::vector<int> new_nodes = new_ring.GetNodes(i);

        std::vector<int> diff;
        CustomerRequest request;
        CustomerRecord c_record;


        for(auto v: old_nodes){
            std::cout<<v<<std::endl;
            if(std::find(new_nodes.begin(), new_nodes.end(), v) == new_nodes.end()){
                diff.push_back(v);
            }
        }


        for(int node: diff){
            request.SetRequest(i,-1, 1);
            ServerClientInterface record = SendToServer(request, RECORD, node);
            c_record = record.record;

            request.SetRequest(i,-1, 3);
            //SendToServer(request, DELETE, node);
        }

        for(int node: new_nodes){
            request.SetRequest(i, c_record.last_order, 2);
            SendToServer(request, INFO, node);
        }
    }
}


