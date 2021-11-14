#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>

class CustomerRequest {
private:
	int customer_id;
	int order_number;
	int request_type;

public:
	CustomerRequest();
	void operator = (const CustomerRequest &order) {
		customer_id = order.customer_id;
		order_number = order.order_number;
        request_type = order.request_type;
	}
	void SetRequest(int cid, int order_num, int type);
	int GetCustomerId();
	int GetOrderNumber();
	int GetRequestType();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

class CustomerRecord{
public:
    int customer_id = -5;
    int last_order = -5;

    CustomerRecord();
    CustomerRecord(int id, int order_no);
    void operator = (const CustomerRecord &info) {
        customer_id = info.customer_id;
        last_order = info.last_order;

    }
    void Marshal(char *buffer);
    void Unmarshal(char *buffer);
    int Size();

};

class HandShaking{
public:
    int message;
    int response;
    HandShaking();
    HandShaking(int m, int res){
        message = m;
        response = res;
    }
    void operator = (const HandShaking &info) {
        message = info.message;
        response = info.response;

    }
    void Marshal(char *buffer);
    void Unmarshal(char *buffer);
    int Size();
};

class LaptopInfo {
private:
	int customer_id;
	int order_number;
	int request_type;
	int engineer_id;
	int admin_id;

public:
	LaptopInfo();
	void operator = (const LaptopInfo &info) {
		customer_id = info.customer_id;
		order_number = info.order_number;
        request_type = info.request_type;
		engineer_id = info.engineer_id;
        admin_id = info.admin_id;
	}
	void SetInfo(int cid, int order_num, int type, int engid, int expid);
	void CopyOrder(CustomerRequest order);
	void SetEngineerId(int id);
	void SetExpertId(int id);

	int GetCustomerId();
	int GetOrderNumber();
	int GetLaptopType();
	int GetEngineerId();
	int GetExpertId();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

struct MapOp{
    int opcode;
    int arg1;
    int arg2;
};

class ReplicationRequest{

public:
    int factory_id;
    int committed_index;
    int last_index;
    MapOp op;

    ReplicationRequest();
    ReplicationRequest(int fac_id, int committed_index, int last_index, MapOp op);
    int Size();
    void Marshal(char *buffer);
    void Unmarshal(char *buffer);
    void operator = (const ReplicationRequest &info) {
        factory_id = info.factory_id;
        committed_index = info.committed_index;
        last_index = info.last_index;
        op = info.op;
    }
    bool IsValid();
};
#endif // #ifndef __MESSAGES_H__
