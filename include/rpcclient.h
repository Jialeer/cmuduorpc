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
	继承RpcChannel需要重写的方法，统一接收rpc client端的rpc方法
	调用，序列化protobuf参数，发送rpc调用请求
	*/
	void CallMethod(const MethodDescriptor* method,
		RpcController* controller, const Message* request,
		Message* response, Closure* done);

private:
	// 连接回调 - 有client连接或者断开连接，该函数会调用
	void onConnection(const TcpConnectionPtr &conn);

	// 消息回调
	void onMessage(const TcpConnectionPtr& conn, Buffer*buf, Timestamp time);
	TcpClient *client_; // bind listen accept
	EventLoop loop_; // 事件循环 epoll
	function<void(const TcpConnectionPtr&)> sendRpcCallBack_;
	function<void(string)> responseCallBack_;
};

#endif