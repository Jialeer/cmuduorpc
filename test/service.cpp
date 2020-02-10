#include "service.pb.h"
#include"rpcserver.h"
#include<iostream>
#include<string>

using namespace std;
//����service.proto����ʽ�����ط��񷢲���rpc����
//service.pb.h�������ɵķ����ඨ�壬���ط���̳и÷����࣬���麯��������д��ʵ�־��幦��
class UserService:public LocalServiceRpc
{
public:
	// ԭ�ȱ��صķ��񷽷�
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
	// ֧��RPCԶ�̵��õķ��񷽷�
	virtual void login(::google::protobuf::RpcController* controller,
		const ::LoginRequest* request,
		::LoginResponse* response,
		::google::protobuf::Closure* done)
	{
		cout << "recv rpcclient call login:->" << request->name() << request->pwd() << endl;
		//ִ�б���ҵ��
		bool isLoginSuccess = log(request->name(), request->pwd());
		//�����д��response��
		response->set_isloginsuccess(isLoginSuccess);
		//ִ�лص�����RpcServer�����ͨ�����緵�ػ�ȥ
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
	//����RpcServer���==��ʵ��һ��RpcServer���󣬽����ط���ע�ᵽ
	RpcServer *rpcserver =RpcServer::getInstance();
	rpcserver->registerServer(new UserService());
	rpcserver->start();
	
	return 0;
}

