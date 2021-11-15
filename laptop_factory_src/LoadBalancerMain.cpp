//
// Created by Egbantan Babatunde on 11/14/21.
//
#include <iostream>

int main(int argc, char *argv[]) {

    int port = atoi(argv(1));
    int number_of_servers = atoi(argv[2]);
    int cache_size = atoi(argv[3]);


    LoadBalancerSocket socket;
    LoadBalancerStub balancerStub;
    std::vector<PeerInfo> primaryServers; //@TODO create new struct for primary servers info

    int offset = 4;

    for(int i = offset; i < number_of_servers; i++){

        int peer_unique_id = atoi(argv[offset++]);
        std::string peer_ip = argv[offset++];
        int peer_port = atoi(argv[offset++]);

        primaryServers.emplace_back(peer_unique_id,  peer_port, peer_ip);
    }

    if (!socket.Init(port)) {
        std::cout << "Socket initialization failed" << std::endl;
        return 0;
    }

    std::vector<std::thread> thread_vector;

    while ((new_socket = socket.Accept())) {

        std::thread balancer_thread(&LoadBalancerStub::BalancerThread,
                                    &balancerStub, std::move(new_socket));
        thread_vector.push_back(std::move(balancer_thread));
    }
    return 0;
}