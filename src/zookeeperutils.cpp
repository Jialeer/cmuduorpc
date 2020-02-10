#include "zookeeperutils.h"
#include "loadxmlconfig.h"

#include <thread>
#include <chrono>
using namespace std;

sem_t ZkClient::_sem;
string ZkClient::_rootNodePath = "/RpcService";

ZkClient::ZkClient() 
	:_zhandle(nullptr)
{
	sem_init(&_sem, 0, 0);
}

void ZkClient::start()
{
	// zookeeper_init����zkServer������session��һ���첽�Ĺ���
	const char *host = XmlConfig::getInstance().getZookeeperHost().c_str();
	_zhandle = zookeeper_init(host, global_watcher, XmlConfig::getInstance().getTimeout(),
		nullptr, nullptr, 0);
	if (_zhandle == nullptr)
	{
		cout << "connecting zookeeper error..." << endl;
		exit(EXIT_FAILURE);
	}

	// �����ȴ����ӳɹ����ڷ���
	sem_wait(&_sem);
	cout << "connecting zookeeper success..." << endl;
}

// zk�����ڵ�
void ZkClient::create(const char *path, const char *data, int datalen, int state)
{
	char path_buffer[128];
	int bufferlen = sizeof(path_buffer);
	// �ж�znode�ڵ��Ƿ���ڣ������ڴ���
	int flag;
	flag = zoo_exists(_zhandle, path, 0, nullptr);
	if (ZNONODE == flag)
	{
		// ��ʾznode�ڵ㲻���ڣ�������ʱ�ڵ�
		flag = zoo_create(_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			cout << "znode create success... path:" << path << endl;
		}
		else
		{
			cout << "flag:" << flag << endl;
			cout << "znode create error... path:" << path << endl;
			exit(EXIT_FAILURE);
		}
	}
}

// get znode�ڵ��ֵ
string ZkClient::get(const char *path)
{
	char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		cout << "get znode error... path:" << path << endl;
		return "";
	}
	else
	{
		return buffer;
	}
}

// ��������
void ZkClient::sendHeartBeat()
{
	// �����̷߳���zk������ά��session
	thread t([&]() {
		for (;;)
		{
			// ����ʱ������Ϊtimeoutʱ���1/3
			int time = XmlConfig::getInstance().getTimeout()*1.0 / 3;
			this_thread::sleep_for(chrono::seconds(time));
			zoo_exists(_zhandle, _rootNodePath.c_str(), 0, nullptr);
		}
	});
	t.detach();
}

// ���ô��RPC����ĸ��ڵ�����
void ZkClient::setRootPath(string path)
{
	_rootNodePath = path;
}

string ZkClient::getRootPath()
{
	return _rootNodePath;
}

// ���watcher��ȫ�ֵĻص�
void ZkClient::global_watcher(zhandle_t *zh, int type,
	int state, const char *path, void *watcherCtx)
{
	cout << "watcher type:" << type << endl;
	cout << "watcher state:" << state << endl;
	cout << "watcher path:" << path << endl;

	if (type == ZOO_SESSION_EVENT) // session�йص��¼�
	{
		if (state == ZOO_CONNECTED_STATE) // session�����ɹ���
		{
			sem_post(&_sem); // ֪ͨ�����߳����ӳɹ�
		}
		else if (state == ZOO_EXPIRED_SESSION_STATE) // session��ʱ��
		{
			
		}
	}
}