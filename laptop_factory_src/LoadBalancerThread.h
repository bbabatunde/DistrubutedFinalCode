//
// Created by Egbantan Babatunde on 11/14/21.
//

#ifndef DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H
#define DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H


#include "MultiPurposeServerSocket.h"
#include "Messages.h"
#include "MultiPurposeClientSocket.h"
#include <map>
#include <vector>

struct ServerInfo{


    int unique_id;
    int port_no;
    std::string peer_ip;
    ServerInfo(){
        unique_id = -1;
        port_no = -1;
        peer_ip = nullptr;
    };
    ServerInfo(int id, int port, std::string ip);
    ServerInfo& operator = (const ServerInfo &info) {
        unique_id = info.unique_id;
        port_no = info.port_no;
        peer_ip = info.peer_ip;

        return *this;
    }
};

class LoadBalancerWorker {
private:

    std::map<int, MultiPurposeClientSocket*> ServersStubsMap;

public:
    LoadBalancerWorker();
    void BalancerThread(std::unique_ptr<MultiPurposeServerSocket> socket);

    void ConnectServers(std::vector<ServerInfo> vector1);

    void ForwardCustomerRequest(char *request);

    void SendIdentification();
};


#endif //DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H
