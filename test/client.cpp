#include "service.pb.h"
#include "rpcclient.h"
#include<iostream>


//�û�����RPC��ܵ���Զ��server
int main()
{
	//LocalServiceRpc_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
	//class RpcClient : public RpcChannel
	/*
	�̳�RpcChannel��Ҫ��д�ķ�����ͳһ����rpc client�˵�rpc����
	���ã����л�protobuf����������rpc��������
	*/
	//virtual void CallMethod(const MethodDescriptor* method,
	//RpcController* controller, const Message* request,
	//	Message* response, Closure* done) = 0;
	LocalServiceRpc_Stub stub(new RpcClient());
	LoginRequest request;
	request.set_name("zhangsan");
	request.set_pwd("88888888");

	LoginResponse response;
	stub.login(nullptr, &request, &response, nullptr);//����CallMethod����rpc��������
	bool loginresponse = response.isloginsuccess();

	cout << "loginresponse:" << loginresponse << endl;
	return 0;
}