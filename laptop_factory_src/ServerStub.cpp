#include "ServerStub.h"
#include <cstring>

ServerStub::ServerStub() {}

void ServerStub::Init(std::unique_ptr<ServerSocket> socket) {
	this->socket = std::move(socket);
}

CustomerRequest ServerStub::ReceiveCustomerRequest() {
	char buffer[32];
	CustomerRequest request;
	if (socket->Recv(buffer, request.Size(), 0)) {
		request.Unmarshal(buffer);
	}
	return request;
}

int ServerStub::ShipLaptop(LaptopInfo info) {
	char buffer[32];
	info.Marshal(buffer);
	return socket->Send(buffer, info.Size(), 0);
}

int ServerStub::ReturnRecord(CustomerRecord info) {
    char buffer[32];
    info.Marshal(buffer);
    return socket->Send(buffer, info.Size(), 0);
}

HandShaking ServerStub::RecieveIdentification() {
    char buffer[24];
    HandShaking id;
    if (socket->Recv(buffer, id.Size(), 0)) {
        id.Unmarshal(buffer);
    }
    return id;
}


ReplicationRequest ServerStub::ReceiveReplicationRequest() {
    char buffer[128];
    ReplicationRequest request;
    int size = request.Size();

    if(socket->Recv(buffer, size, 0)){
        request.Unmarshal(buffer);
    }
    return request;
}

void ServerStub::SendReplicationResponse() {
    char buffer[32];
    int response = 1;
    int size = sizeof(response);

    memcpy(buffer, &response, sizeof(response));
    socket->Send(buffer, size, 0);
}

int ServerStub::InitToBackup(std::string ip, int port) {

    int result = socket2.InitToBackup(ip, port);

    if(result){
        HandShaking id(1, -1);
        result = SendIdentification(id);
    }
    return  result;
}

int ServerStub::SendIdentification(HandShaking id) {
    char buffer[32];
    int size = id.Size();

    id.Marshal(buffer);
    if(socket2.Send(buffer, size, 0)){
        return  1;
    }else{
        return 0;
    }
}

int ServerStub::SendReplicationRequest(ReplicationRequest request) {
    char buffer[128];
    int size = request.Size();

    request.Marshal(buffer);

    int response = 0;
    if(socket2.Send(buffer, size, 0)){

        size = sizeof(response);
        if(socket2.Recv(buffer, size, 0)){
            memcpy(&response, buffer, sizeof(response));
        }else{
            return -1;
        }
    }else{
        return -1;
    }
    return response;
}


