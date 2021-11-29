//
// Created by Egbantan Babatunde on 11/14/21.
//
#include <iostream>
#include "MultiPurposeServerSocket.h"
#include "LoadBalancerThread.h"
#include <thread>

#include <vector>
#include <string>
int main(int argc, char *argv[]) {

    int port = atoi(argv[1]);;
    int number_of_servers = atoi(argv[2]);
    int cache_size = atoi(argv[3]);


    MultiPurposeServerSocket socket;
    LoadBalancerWorker balancer;
    std::vector<ServerInfo> primaryServers;
    std::unique_ptr<MultiPurposeServerSocket> new_socket;
    balancer.SetCacheSize(cache_size);
    int offset = 4;

    for(int i = 0; i < number_of_servers; i++){

        int peer_unique_id = atoi(argv[offset++]); //@TODO could internal and not from user
        std::string peer_ip = argv[offset++];
        int peer_port = atoi(argv[offset++]);

        primaryServers.emplace_back(peer_unique_id,  peer_port, peer_ip);
    }

    if (!socket.Init(port)) {
        std::cout << "Socket initialization failed" << std::endl;
        return 0;
    }

    balancer.ConnectServers(primaryServers);

    std::vector<std::thread> thread_vector;

    while ((new_socket = socket.Accept())) {
        std::thread worker_thread(&LoadBalancerWorker::BalancerThread,
                                  &balancer, std::move(new_socket));
        thread_vector.push_back(std::move(worker_thread));
    }
    return 0;
}