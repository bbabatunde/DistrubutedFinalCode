//
// Created by Egbantan Babatunde on 11/14/21.
//

#include "LoadBalancerStub.h"
#include <iostream>
void LoadBalancerStub::Init(std::unique_ptr<MultiPurposeServerSocket> socket) {
    this->server_socket = std::move(socket);

}

char *LoadBalancerStub::ReadClientBuffer() {
    char buffer[32];

    if(server_socket->Recv(buffer, 32,0)){
        std::cout<<"buffer"<<std::endl;

        return buffer;

    }
    return nullptr;
}
