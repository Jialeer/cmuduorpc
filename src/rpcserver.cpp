#include "rpcserver.h"
#include "rpcmeta.pb.h"
#include "loadxmlconfig.h"
#include <thread>
using namespace std;


//先将static变量初始化，赋值一下为nullptr方便后面实例化
RpcServer *RpcServer::rpcServer_ = nullptr;
//获取RpcServer的一个实例
RpcServer* RpcServer::getInstance()
{
	//RpcServer对象未实例化
	if (rpcServer_ == nullptr)
	{
		//对象的变量（对象）不能出作用域析构，所以变量要为static静态变量
		static EventLoop loop;
		static InetAddress addr(XmlConfig::getInstance().getRpcServerIp(), 
			XmlConfig::getInstance().getRpcServerPort());
		rpcServer_ = new RpcServer(&loop, addr, "RpcServer");
	}
	return rpcServer_;
}
//RpcServer的构造函数
RpcServer::RpcServer(EventLoop *loop,
	const InetAddress &addr, 
	const string& name):server_(loop,addr,name),loop_(loop)//默认参数列表
{
	//发送连接和处理消息是异步进行的，所以利用绑定器和回调函数
	//绑定连接回调，设置有连接事件时，回调到RpcServer::onConnection函数执行具体事件
	server_.setConnectionCallback(bind(&RpcServer::onConnection, this, _1));
	//绑定消息回调，设置有消息事件时，回调到pcServer::onMessage函数执行具体事件
	server_.setMessageCallback(bind(&RpcServer::onMessage, this, _1, _2, _3));
	server_.setThreadNum(4);
	//启动zkclient客户端
	zkclient_.start();
	//发送心跳，与zkServer之间维持一个session会话，每1/3timeout发送一下
	zkclient_.sendHeartBeat();
}

void RpcServer::start()
{
	//启动epoll循环监听
	server_.start();
	loop_->loop();
}

//google::protobuf::Service基于协议缓冲区的RPC服务的抽象基接口。
//服务它们本身是抽象接口（由服务器或作为接口实现存根）但它们是这个基本接口的子类。
//这种方法接口可用于调用服务的方法，而不必知道它在编译时的确切类型（类似于反射）。

//把本地服务注册到自己维护的服务列表中
void RpcServer::registerServer(google::protobuf::Service *service)
{
	//包装服务对象和方法调用的一个结构，用来存储在RpcServer的服务列表中
	ServerInfo serinfo;
	//包装填充这个结构
	auto server = service->GetDescriptor();
	string server_name = server->name();
	int method_count = server->method_count();

	//zookeeper上存储本地服务器的路径
	string rootpath = zkclient_.getRootPath();
	rootpath += "/" + server_name;
	//在zookeeper上创建一个服务器节点
	zkclient_.create(rootpath.c_str(), nullptr, -1);
	//在服务器节点下创建字节点（服务）并且填充结构
	for (int i = 0; i < method_count; ++i)
	{
		auto method = server->method(i);
		string method_name = method->name();
		serinfo.methodMap_.insert({method_name, method});

		string methodpath;
		methodpath += (rootpath + "/" + method_name);
		zkclient_.create(methodpath.c_str(), server_.ipPort().c_str(),
			server_.ipPort().size(), ZOO_EPHEMERAL);
	}
	serinfo.service_ = service;
	//将服务器结构放入unordered_map服务列表中
	serverMap_.insert({ server_name,serinfo });
}
//处理新连接事件
void RpcServer::onConnection(const TcpConnectionPtr &con)
{
	//客户端断开事件
	if (!con->connected())
	{
		con->shutdown();
	}
}
//处理新消息事件
void RpcServer::onMessage(const TcpConnectionPtr& con,
	Buffer *buf,
	Timestamp time)
{
	//4bite(请求头的字节长度)+service_name/method_name+args(请求参数)
	string request_str = buf->retrieveAllAsString();
	int head_size = 0;
	// insert 按字节插入的数据，通过copy按字节拷贝出来进行读取
	request_str.copy((char*)&head_size, sizeof(int), 0);
	string head_string = request_str.substr(4, head_size);
	rpc_header head;
	head.ParseFromString(head_string);
	string server_name = head.service_name();
	string method_name = head.method_name();
	string args_str = request_str.substr(4 + head_size);

	cout << "=======================================" << endl;
	cout << "head_size:" << head_size << endl;
	cout << "head_string:" << head_string << endl;
	cout << "server_name:" << server_name << endl;
	cout << "method_name:" << method_name << endl;
	cout << "args_str:" << args_str << endl;
	cout << "=======================================" << endl;

	// 根据service_name+method_name 分发方法调用
	ServerInfo serinfo = serverMap_[server_name];
	auto service = serinfo.service_;
	auto method = serinfo.methodMap_[method_name];
	// Message*基类指针类型
	auto request = service->GetRequestPrototype(method).New();
	auto response = service->GetResponsePrototype(method).New();
	request->ParseFromString(args_str);

	// response
	auto done = google::protobuf::NewCallback<RpcServer, const TcpConnectionPtr&, google::protobuf::Message*>
		(this, &RpcServer::sendRpcResponse, con, response);
	//做本地业务写入response
	service->CallMethod(method, nullptr, request, response, done);
}


void RpcServer::sendRpcResponse(const TcpConnectionPtr &con,
	google::protobuf::Message *response)
{
	//将结果protobuf序列化后发出
	string res_str;
	response->SerializeToString(&res_str);
	con->send(res_str);
	//服务器主动断开连接
	con->shutdown();
}


