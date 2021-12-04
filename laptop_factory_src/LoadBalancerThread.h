//
// Created by Egbantan Babatunde on 11/14/21.
//

#ifndef DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H
#define DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H



#include "LoadBalancerRing.h"
#include "LoadBalancerStub.h"
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <iterator>

class LoadBalancerWorker {
private:

    std::map<int, MultiPurposeClientSocket*> ServersStubsMap;
    LoadBalancerRing ring;
    LoadBalancerRing old_ring;

public:
    LoadBalancerWorker();
    void BalancerThread(std::unique_ptr<MultiPurposeServerSocket> socket);

    void InitRing(const std::vector<ServerInfo>& vector1,  LoadBalancerRing ring);

    void SendIdentification(int id);

    ServerClientInterface SendToServer(CustomerRequest request, ServerClientInterfaceOp operation, int server);


    void CustomerThread(LoadBalancerStub &&stub);

    void SysAdminThread(LoadBalancerStub &&stub);

    void Migrate();
};


#endif //DISTRUBUTEDFINALCODE_LOADBALANCERTHREAD_H
