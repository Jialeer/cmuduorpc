#ifndef RPCSERVER_H
#define RPCSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <functional>
#include <unordered_map>
#include <string>
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;
#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "zookeeperutils.h"

/*
����C++��protobufʵ�ֵ�RPC��ܵķ���˿�ܴ��룬�������ҵ�����
*/
class RpcServer
{
public:
	// ��ȡΨһʵ���ķ���
	static RpcServer* getInstance();
	// epoll_ctl => listenfd => epoll
	void start();
	// ���û����ø÷�������RpcServerע��RPC����
	void registerServer(google::protobuf::Service *service);
private:
	// ��ʼ��
	RpcServer(EventLoop *loop, const InetAddress &addr, const string& name);
	// ��������
	void onConnection(const TcpConnectionPtr &con);
	// ������ղ�������Ӧ��Ϣ�� 
	void onMessage(const TcpConnectionPtr& con,
		Buffer *buf,
		Timestamp time);
	// do->Run���� ��Ӧ�Ļص�
	void sendRpcResponse(const TcpConnectionPtr &con,
		google::protobuf::Message *response);

	TcpServer server_;
	EventLoop *loop_;
	// unordered_map ��¼�����Լ������Ӧ�����з���
	// ��װ�������ͷ�������
	struct ServerInfo
	{
		google::protobuf::Service *service_; // ָ��������
		unordered_map<string, const google::protobuf::MethodDescriptor*> methodMap_; // ��������������ķ���
	};
	// ����������   -   ServerInfo
	unordered_map<string, ServerInfo> serverMap_;
	// ���Ψһ��ʵ��
	static RpcServer *rpcServer_;
	// zk�Ŀͻ���ʵ������������zk�ͻ����Լ�����zonde�ڵ�
	ZkClient zkclient_;
};


#endif
