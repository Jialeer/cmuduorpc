#include "rpcclient.h"
#include "rpcmeta.pb.h"
#include <thread>

RpcClient::RpcClient()
	:client_(nullptr)
{

}

RpcClient::~RpcClient()
{
	delete client_;
	client_ = nullptr;
}
//rpc client端所有的rpc服务方法调用最终都到达这里
//rpcclient 接受到所有本地请求后都汇集到这里，
//然后组装request，response待连接成功后发送给rpcserver
void RpcClient::CallMethod(const MethodDescriptor* method,
	RpcController* controller, const Message* request,
	Message* response, Closure* done)
{
	auto service = method->service();
	string server_name = service->name();
	string method_name = method->name();

	ZkClient zk;
	zk.start();
	string path = ZkClient::getRootPath() + "/" + server_name + "/" + method_name;
	//string get(const char *path);
	string host = zk.get(path.c_str());
	//c_str()函数返回一个指向正规C字符串的指针, 内容与本string串相同. 
	//这是为了与c语言兼容，在c语言中没有string类型，故必须通过string类对象
	//的成员函数c_str()把string 对象转换成c中的字符串样式
	if (host.size() == 0)
	{
		cout << "cannot find server!" << endl;
		return;
	}

	//组装请求字符串
	rpc_header head;
	head.set_method_name(method_name);
	head.set_service_name(server_name);
	//服务方法头
	string rpc_head_str;
	//protobuf序列化
	head.SerializeToString(&rpc_head_str);
	int head_size = rpc_head_str.size();
	//参数字符串
	string rpc_args_str;
	//protobuf序列化
	request->SerializeToString(&rpc_args_str);

	//最终发生的请求字符串
	string request_send_str;
	request_send_str.insert(0, string((char*)&head_size, 4));//(1)size
	request_send_str += rpc_head_str;						 //(2)service/method
	request_send_str += rpc_args_str;						 //(3)args

	cout << "============================================" << endl;
	cout << "header_size:" << head_size << endl;
	cout << "rpc_header_str:" << rpc_head_str << endl;
	cout << "service_name:" << server_name << endl;
	cout << "method_name:" << method_name << endl;
	cout << "rpc_args_str:" << rpc_args_str << endl;
	cout << "============================================" << endl;
	//预设一个回调，当成功连接RpcServer时发送RPC请求
	sendRpcCallBack_ = [&](const TcpConnectionPtr &con)
	{
		con->send(request_send_str);
	};
	//预设一个回调，当异步接受到rpcserver响应的时候调用
	responseCallBack_ = [&](string message)
	{
		response->ParseFromString(message);
	};
	int idx = host.find(':');
	string ip = host.substr(0, idx);
	unsigned short port = atoi(host.substr(idx + 1).c_str());
	client_ = new TcpClient(&loop_, InetAddress(ip, port), "");
	//绑定连接事件的通知函数
	client_->setConnectionCallback(bind(&RpcClient::onConnection, this, _1));
	//绑定消息事件的通知函数
	client_->setMessageCallback(bind(&RpcClient::onMessage, this, _1, _2, _3));
	client_->connect();
	loop_.loop();
}

//连接回调，有连接事件发生时，该函数会调用
void RpcClient::onConnection(const TcpConnectionPtr &con)
{
	if (con->connected())//成功连接RpcServer
	{
		sendRpcCallBack_(con);
	}
	else
	{
		con->shutdown();
		loop_.quit();
	}
}

//消息回调，接受到Rpcserver处理完成的response消息后调用
void RpcClient::onMessage(const TcpConnectionPtr& conn, Buffer*buf, Timestamp time)
{
	//接收rpc远程调用方法的返回值
	string message = buf->retrieveAllAsString();
	//给用户调用通知RPC调用结果
	responseCallBack_(message);
}