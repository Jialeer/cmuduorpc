#include "service.pb.h"
#include"rpcserver.h"
#include<iostream>
#include<string>

using namespace std;
//根据service.proto的形式将本地服务发布成rpc服务
//service.pb.h会有生成的服务类定义，本地服务继承该服务类，对虚函数进行重写，实现具体功能
class UserService:public LocalServiceRpc
{
public:
	// 原先本地的服务方法
	bool log(string name, string pwd)
	{
		cout << "call UserService::login->";
		cout << "name:" << name << " ";
		cout << "pwd:" << pwd << endl;
		return true;
	}
	bool reg(int id, string name, string pwd, string tel)
	{
		cout << "call UserService::reg->";
		cout << "id:" << id << " ";
		cout << "name:" << name << " ";
		cout << "pwd:" << pwd << endl;
		cout << "tel:" << tel << endl;
	}
	// 支持RPC远程调用的服务方法
	virtual void login(::google::protobuf::RpcController* controller,
		const ::LoginRequest* request,
		::LoginResponse* response,
		::google::protobuf::Closure* done)
	{
		cout << "recv rpcclient call login:->" << request->name() << request->pwd() << endl;
		//执行本地业务
		bool isLoginSuccess = log(request->name(), request->pwd());
		//将结果写道response中
		response->set_isloginsuccess(isLoginSuccess);
		//执行回调，由RpcServer将结果通过网络返回回去
		done->Run();
	}
	virtual void reg(::google::protobuf::RpcController* controller,
		const ::RegRequest* request,
		::RegResponse* response,
		::google::protobuf::Closure* done)
	{

	}

};


int main()
{
	//调用RpcServer框架==》实例一个RpcServer对象，将本地服务注册到
	RpcServer *rpcserver =RpcServer::getInstance();
	rpcserver->registerServer(new UserService());
	rpcserver->start();
	
	return 0;
}

