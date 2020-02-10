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
//rpc client�����е�rpc���񷽷��������ն���������
//rpcclient ���ܵ����б�������󶼻㼯�����
//Ȼ����װrequest��response�����ӳɹ����͸�rpcserver
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
	//c_str()��������һ��ָ������C�ַ�����ָ��, �����뱾string����ͬ. 
	//����Ϊ����c���Լ��ݣ���c������û��string���ͣ��ʱ���ͨ��string�����
	//�ĳ�Ա����c_str()��string ����ת����c�е��ַ�����ʽ
	if (host.size() == 0)
	{
		cout << "cannot find server!" << endl;
		return;
	}

	//��װ�����ַ���
	rpc_header head;
	head.set_method_name(method_name);
	head.set_service_name(server_name);
	//���񷽷�ͷ
	string rpc_head_str;
	//protobuf���л�
	head.SerializeToString(&rpc_head_str);
	int head_size = rpc_head_str.size();
	//�����ַ���
	string rpc_args_str;
	//protobuf���л�
	request->SerializeToString(&rpc_args_str);

	//���շ����������ַ���
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
	//Ԥ��һ���ص������ɹ�����RpcServerʱ����RPC����
	sendRpcCallBack_ = [&](const TcpConnectionPtr &con)
	{
		con->send(request_send_str);
	};
	//Ԥ��һ���ص������첽���ܵ�rpcserver��Ӧ��ʱ�����
	responseCallBack_ = [&](string message)
	{
		response->ParseFromString(message);
	};
	int idx = host.find(':');
	string ip = host.substr(0, idx);
	unsigned short port = atoi(host.substr(idx + 1).c_str());
	client_ = new TcpClient(&loop_, InetAddress(ip, port), "");
	//�������¼���֪ͨ����
	client_->setConnectionCallback(bind(&RpcClient::onConnection, this, _1));
	//����Ϣ�¼���֪ͨ����
	client_->setMessageCallback(bind(&RpcClient::onMessage, this, _1, _2, _3));
	client_->connect();
	loop_.loop();
}

//���ӻص����������¼�����ʱ���ú��������
void RpcClient::onConnection(const TcpConnectionPtr &con)
{
	if (con->connected())//�ɹ�����RpcServer
	{
		sendRpcCallBack_(con);
	}
	else
	{
		con->shutdown();
		loop_.quit();
	}
}

//��Ϣ�ص������ܵ�Rpcserver������ɵ�response��Ϣ�����
void RpcClient::onMessage(const TcpConnectionPtr& conn, Buffer*buf, Timestamp time)
{
	//����rpcԶ�̵��÷����ķ���ֵ
	string message = buf->retrieveAllAsString();
	//���û�����֪ͨRPC���ý��
	responseCallBack_(message);
}