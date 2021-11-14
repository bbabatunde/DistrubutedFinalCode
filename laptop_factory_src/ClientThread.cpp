#include "ClientThread.h"
#include "Messages.h"

#include <iostream>

ClientThreadClass::ClientThreadClass() {}

void ClientThreadClass::
ThreadBody(std::string ip, int port, int id, int orders, int type) {
	customer_id = id;
	num_orders = orders;
    request_type = type;

	if (!stub.Init(ip, port)) {
		std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
		return;
	}
	for (int i = 0; i < num_orders; i++) {
		CustomerRequest request;
		LaptopInfo laptop;
        CustomerRecord record;


		timer.Start();
        if(type == 1) {
            request.SetRequest(customer_id, i, request_type);
            laptop = stub.Order(request);
        }
        else if(type == 2 || type == 3) {
            request.SetRequest(i, -1, 2);
            record = stub.ReadRecord(request);
            if(record.customer_id == -5)
                return;
        }

        if(type == 3 && record.customer_id != -1) {
            std::cout << record.customer_id <<"\t"<<record.last_order<<std::endl;
        }
		timer.EndAndMerge();

		if (type == 1 && !laptop.IsValid()) {
			return;
		}
	}
}

ClientTimer ClientThreadClass::GetTimer() {
	return timer;	
}

