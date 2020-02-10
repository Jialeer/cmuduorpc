#include "rpcserver.h"
#include "rpcmeta.pb.h"
#include "loadxmlconfig.h"
#include <thread>
using namespace std;


//�Ƚ�static������ʼ������ֵһ��Ϊnullptr�������ʵ����
RpcServer *RpcServer::rpcServer_ = nullptr;
//��ȡRpcServer��һ��ʵ��
RpcServer* RpcServer::getInstance()
{
	//RpcServer����δʵ����
	if (rpcServer_ == nullptr)
	{
		//����ı��������󣩲��ܳ����������������Ա���ҪΪstatic��̬����
		static EventLoop loop;
		static InetAddress addr(XmlConfig::getInstance().getRpcServerIp(), 
			XmlConfig::getInstance().getRpcServerPort());
		rpcServer_ = new RpcServer(&loop, addr, "RpcServer");
	}
	return rpcServer_;
}
//RpcServer�Ĺ��캯��
RpcServer::RpcServer(EventLoop *loop,
	const InetAddress &addr, 
	const string& name):server_(loop,addr,name),loop_(loop)//Ĭ�ϲ����б�
{
	//�������Ӻʹ�����Ϣ���첽���еģ��������ð����ͻص�����
	//�����ӻص��������������¼�ʱ���ص���RpcServer::onConnection����ִ�о����¼�
	server_.setConnectionCallback(bind(&RpcServer::onConnection, this, _1));
	//����Ϣ�ص�����������Ϣ�¼�ʱ���ص���pcServer::onMessage����ִ�о����¼�
	server_.setMessageCallback(bind(&RpcServer::onMessage, this, _1, _2, _3));
	server_.setThreadNum(4);
	//����zkclient�ͻ���
	zkclient_.start();
	//������������zkServer֮��ά��һ��session�Ự��ÿ1/3timeout����һ��
	zkclient_.sendHeartBeat();
}

void RpcServer::start()
{
	//����epollѭ������
	server_.start();
	loop_->loop();
}

//google::protobuf::Service����Э�黺������RPC����ĳ�����ӿڡ�
//�������Ǳ����ǳ���ӿڣ��ɷ���������Ϊ�ӿ�ʵ�ִ��������������������ӿڵ����ࡣ
//���ַ����ӿڿ����ڵ��÷���ķ�����������֪�����ڱ���ʱ��ȷ�����ͣ������ڷ��䣩��

//�ѱ��ط���ע�ᵽ�Լ�ά���ķ����б���
void RpcServer::registerServer(google::protobuf::Service *service)
{
	//��װ�������ͷ������õ�һ���ṹ�������洢��RpcServer�ķ����б���
	ServerInfo serinfo;
	//��װ�������ṹ
	auto server = service->GetDescriptor();
	string server_name = server->name();
	int method_count = server->method_count();

	//zookeeper�ϴ洢���ط�������·��
	string rootpath = zkclient_.getRootPath();
	rootpath += "/" + server_name;
	//��zookeeper�ϴ���һ���������ڵ�
	zkclient_.create(rootpath.c_str(), nullptr, -1);
	//�ڷ������ڵ��´����ֽڵ㣨���񣩲������ṹ
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
	//���������ṹ����unordered_map�����б���
	serverMap_.insert({ server_name,serinfo });
}
//�����������¼�
void RpcServer::onConnection(const TcpConnectionPtr &con)
{
	//�ͻ��˶Ͽ��¼�
	if (!con->connected())
	{
		con->shutdown();
	}
}
//��������Ϣ�¼�
void RpcServer::onMessage(const TcpConnectionPtr& con,
	Buffer *buf,
	Timestamp time)
{
	//4bite(����ͷ���ֽڳ���)+service_name/method_name+args(�������)
	string request_str = buf->retrieveAllAsString();
	int head_size = 0;
	// insert ���ֽڲ�������ݣ�ͨ��copy���ֽڿ����������ж�ȡ
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

	// ����service_name+method_name �ַ���������
	ServerInfo serinfo = serverMap_[server_name];
	auto service = serinfo.service_;
	auto method = serinfo.methodMap_[method_name];
	// Message*����ָ������
	auto request = service->GetRequestPrototype(method).New();
	auto response = service->GetResponsePrototype(method).New();
	request->ParseFromString(args_str);

	// response
	auto done = google::protobuf::NewCallback<RpcServer, const TcpConnectionPtr&, google::protobuf::Message*>
		(this, &RpcServer::sendRpcResponse, con, response);
	//������ҵ��д��response
	service->CallMethod(method, nullptr, request, response, done);
}


void RpcServer::sendRpcResponse(const TcpConnectionPtr &con,
	google::protobuf::Message *response)
{
	//�����protobuf���л��󷢳�
	string res_str;
	response->SerializeToString(&res_str);
	con->send(res_str);
	//�����������Ͽ�����
	con->shutdown();
}


