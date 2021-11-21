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





class LoadBalancerWorker {
private:

    std::map<int, MultiPurposeClientSocket*> ServersStubsMap;

public:
    LoadBalancerWorker();
    void BalancerThread(std::unique_ptr<MultiPurposeServerSocket> socket);

    void ConnectServers(std::vector<ServerInfo> vector1);

    void SendIdentification();

    ServerClientInterface SendToServer(CustomerRequest request, ServerClientInterfaceOp operation);
};


#endif //DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H
