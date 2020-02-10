#include "service.pb.h"
#include "rpcclient.h"
#include<iostream>


//用户利用RPC框架调用远程server
int main()
{
	//LocalServiceRpc_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
	//class RpcClient : public RpcChannel
	/*
	继承RpcChannel需要重写的方法，统一接收rpc client端的rpc方法
	调用，序列化protobuf参数，发送rpc调用请求
	*/
	//virtual void CallMethod(const MethodDescriptor* method,
	//RpcController* controller, const Message* request,
	//	Message* response, Closure* done) = 0;
	LocalServiceRpc_Stub stub(new RpcClient());
	LoginRequest request;
	request.set_name("zhangsan");
	request.set_pwd("88888888");

	LoginResponse response;
	stub.login(nullptr, &request, &response, nullptr);//调用CallMethod发送rpc调用请求
	bool loginresponse = response.isloginsuccess();

	cout << "loginresponse:" << loginresponse << endl;
	return 0;
}