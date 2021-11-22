//
// Created by Egbantan Babatunde on 11/14/21.
//

#include "LoadBalancerStub.h"
#include <iostream>
void LoadBalancerStub::Init(std::unique_ptr<MultiPurposeServerSocket> socket) {
    this->server_socket = std::move(socket);

}

CustomerRequest LoadBalancerStub::ReceiveCustomerRequest() {
    char buffer[32];
    CustomerRequest request;
    if (server_socket->Recv(buffer, request.Size(), 0)) {
        request.Unmarshal(buffer);
    }
    return request;
}

int LoadBalancerStub::Ship(ServerClientInterface info,ServerClientInterfaceOp operation) {
    char buffer[32];

    if(operation == INFO){
        info.info.Marshal(buffer);
        return server_socket->Send(buffer, info.info.Size(), 0);
    }else{
        info.record.Marshal(buffer);
        return server_socket->Send(buffer, info.record.Size(), 0);
    }


}
