#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <functional>
#include <string>
#include <iostream>
#include <semaphore.h>
#include "service.pb.h"
#include "zookeeperutils.h"

using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;
using namespace google::protobuf;


// stub(RpcChannel)
class RpcClient : public RpcChannel
{
public:
	RpcClient();
	~RpcClient();

	/*
	�̳�RpcChannel��Ҫ��д�ķ�����ͳһ����rpc client�˵�rpc����
	���ã����л�protobuf����������rpc��������
	*/
	void CallMethod(const MethodDescriptor* method,
		RpcController* controller, const Message* request,
		Message* response, Closure* done);

private:
	// ���ӻص� - ��client���ӻ��߶Ͽ����ӣ��ú��������
	void onConnection(const TcpConnectionPtr &conn);

	// ��Ϣ�ص�
	void onMessage(const TcpConnectionPtr& conn, Buffer*buf, Timestamp time);
	TcpClient *client_; // bind listen accept
	EventLoop loop_; // �¼�ѭ�� epoll
	function<void(const TcpConnectionPtr&)> sendRpcCallBack_;
	function<void(string)> responseCallBack_;
};

#endif