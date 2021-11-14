#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <map>
#include <vector>

#include "Messages.h"
#include "ServerSocket.h"
#include "ServerStub.h"

struct ExpertRequest {
	LaptopInfo laptop;
	std::promise<LaptopInfo> prom;
};



struct PeerInfo{


    int unique_id;
    int port_no;
    std::string peer_ip;
    PeerInfo(){
        unique_id = -1;
        port_no = -1;
        peer_ip = nullptr;
    };
    PeerInfo(int id, int port, std::string ip);
    PeerInfo& operator = (const PeerInfo &info) {
        unique_id = info.unique_id;
        port_no = info.port_no;
        peer_ip = info.peer_ip;

        return *this;
    }
};


class LaptopFactory {
private:
	std::queue<std::unique_ptr<ExpertRequest>> erq;
	std::mutex erq_lock;
	std::condition_variable erq_cv;

	LaptopInfo CreateLaptop(CustomerRequest order, int engineer_id);
	LaptopInfo PerformAdminstrative(CustomerRequest order, int engineer_id);

    std::mutex customer_record_lock;
    std::mutex map_record_lock;

    std::map<int, int> customer_record;
    std::vector<MapOp> smr_log;
    std::vector<PeerInfo> peer_info;
    std::vector<ServerStub*> peer_stubs;
    std::vector<ServerStub*> zombies_peer_stubs;
    std::map<ServerStub*, PeerInfo> peer_map;

    int last_index = -1;
    int committed_index = -1;
    int primary_id = -1;
    int factory_id;

    bool is_switched = false;
    bool is_backup = false;
public:

	void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
	void ProductionAdminThread(int id);

    CustomerRecord ReadRecord(CustomerRequest request);
    void SetFactoryId(int id){
        factory_id = id;
    }

    int GetFactoryId();

    int GetPrimaryId();

    void SetPrimaryId(int i);

    bool Replicate(MapOp op);

    void EngineerRole(ServerStub stub, int id);

    void IdleFactoryAdminRole(ServerStub stub);

    void ConnectToBackups();

    void SetPeerInfo(std::vector<PeerInfo>& info);

    int UpdateBackup(ServerStub *pStub);
};

#endif // end of #ifndef __SERVERTHREAD_H__

