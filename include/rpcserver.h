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
基于C++和protobuf实现的RPC框架的服务端框架代码，不能添加业务代码
*/
class RpcServer
{
public:
	// 获取唯一实例的方法
	static RpcServer* getInstance();
	// epoll_ctl => listenfd => epoll
	void start();
	// 让用户调用该方法，向RpcServer注册RPC服务
	void registerServer(google::protobuf::Service *service);
private:
	// 初始化
	RpcServer(EventLoop *loop, const InetAddress &addr, const string& name);
	// 负责连接
	void onConnection(const TcpConnectionPtr &con);
	// 负责接收并发送响应消息的 
	void onMessage(const TcpConnectionPtr& con,
		Buffer *buf,
		Timestamp time);
	// do->Run（） 对应的回调
	void sendRpcResponse(const TcpConnectionPtr &con,
		google::protobuf::Message *response);

	TcpServer server_;
	EventLoop *loop_;
	// unordered_map 记录服务以及服务对应的所有方法
	// 包装服务对象和方法调用
	struct ServerInfo
	{
		google::protobuf::Service *service_; // 指向服务对象
		unordered_map<string, const google::protobuf::MethodDescriptor*> methodMap_; // 服务对象里面具体的方法
	};
	// 服务类名字   -   ServerInfo
	unordered_map<string, ServerInfo> serverMap_;
	// 设计唯一的实例
	static RpcServer *rpcServer_;
	// zk的客户端实例，用来启动zk客户端以及创造zonde节点
	ZkClient zkclient_;
};


#endif
