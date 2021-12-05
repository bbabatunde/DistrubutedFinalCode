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

void LoadBalancerWorker::InitRing(const std::vector<ServerInfo>& vector1, LoadBalancerRing ring1) {
    old_ring = ring;
    ring = std::move(ring1);

    for(auto v: vector1){
        ServersStubsMap[v.unique_id] = std::move(new MultiPurposeClientSocket);
        ServersStubsMap[v.unique_id]->Init( v.peer_ip,v.port_no);
        SendIdentification(v.unique_id);

    }
    Migrate();
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
        }else {
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
                if(algorithm)
                    nodes =  ring.GetNodes(customer_id);
                else
                    nodes = GetRandAssignedServer(customer_id);


                for(auto node: nodes){
                    ServerClientInterface temp_info = SendToServer(request, INFO, node);
                    if(temp_info.info.GetCustomerId() == -1){
                        ring.InvalidateNode(node);
                    }else{
                        info = temp_info;
                    }
                }

                inter.info = info;
                stub.Ship(inter, INFO);

                //invalidation after successful write
                if(cache.find(customer_id) != cache.end()){
                    cache[customer_id] = -1;
                }
                break;

            case 2:

                customer_id = request.GetCustomerId();

                if(algorithm)
                    nodes =  ring.GetNodes(customer_id);
                else
                    nodes = GetRandAssignedServer(customer_id);

                random = rand() % (nodes.size());

                hashed_server = nodes[random];
                record = SendToServer(request, RECORD, hashed_server);



                //use cache
                if(cache.find(customer_id) != cache.end() && cache[customer_id] != -1){
                    record = CustomerRecord(customer_id, cache[customer_id]);

                }else{ // go to server
                    record = SendToServer(request, RECORD, hashed_server);
                }



                inter.record = record;
                stub.Ship(inter, RECORD);
                cache[record.customer_id] = record.last_order;
                break;



            //@TODO read all cache content baba from client(5)
            case 5:
                stub.ShipCacheToClient(cache);

            default:
                std::cout << "Undefined request_type type: "
                          << request_type << std::endl;


        }
    }
}

void LoadBalancerWorker::SysAdminThread(LoadBalancerStub &&stub) {
    int request_type;
    std::vector<ServerInfo> servers;
    size_t count;
    AdminRequest request;

    while (true) {

         request = stub.ReceiveAdminRequest();
        if (request.GetRequestType()== -1) {
            break;
        }


        request_type = request.GetRequestType();
        switch (request_type) {

            case 1:
                servers.push_back(request.GetServerInfo());
                break;
            case 2:
                 count = servers.size() + ring.GetServersCount();

                LoadBalancerRing new_ring( count, ring.GetNumberofReplicas());
                InitRing(servers, new_ring);
                break;


        }
    }
}

void LoadBalancerWorker::Migrate() {

    for(int i = 0; i <= old_ring.GetDataCount(); i++){

        std::vector<int> old_nodes = old_ring.GetNodes(i);
        std::vector<int> new_nodes = ring.GetNodes(i);

        sort(old_nodes.begin(), old_nodes.end());
        sort(new_nodes.begin(), new_nodes.end());

        std::vector<int> diff;
        std::vector<int> set_diff;
        std::vector<int> create_laptop_set;

        CustomerRequest request;

        std::set_difference(new_nodes.begin(), new_nodes.end(), old_nodes.begin(), old_nodes.end(),
                            std::inserter(create_laptop_set, create_laptop_set.end()));


        //get a record from first node
        request.SetRequest(i,-1, 2);
        ServerClientInterface record = SendToServer(request, RECORD, old_nodes[0]);

        //create laptop in new nodes
        for(int node: create_laptop_set){
            request.SetRequest(i,record.record.last_order, 1);
            SendToServer(request, INFO, node);
        }

        //DELETE from old nodes
        std::set<int> delete_laptop_set;
        std::set_difference(old_nodes.begin(), old_nodes.end(), new_nodes.begin(), new_nodes.end(),
                            std::inserter(delete_laptop_set, delete_laptop_set.end()));

        for(int node: delete_laptop_set){
            request.SetRequest(i,3, 3);
            SendToServer(request, RECORD, node);
        }

    }
}

void LoadBalancerWorker::SetAlgorithm(int i) {
    algorithm = i;
}

std::vector<int> LoadBalancerWorker::GetRandAssignedServer(int id) {
    int server;
    if(randServerMap.find(id) == randServerMap.end()){
        server  = rand() % ring.GetServersCount();
        randServerMap[id] = server;

    }

    return std::vector<int>{randServerMap[id]};
}


