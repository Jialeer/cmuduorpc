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
	// zookeeper_init连接zkServer创建好session是一个异步的过程
	const char *host = XmlConfig::getInstance().getZookeeperHost().c_str();
	_zhandle = zookeeper_init(host, global_watcher, XmlConfig::getInstance().getTimeout(),
		nullptr, nullptr, 0);
	if (_zhandle == nullptr)
	{
		cout << "connecting zookeeper error..." << endl;
		exit(EXIT_FAILURE);
	}

	// 阻塞等待连接成功，在返回
	sem_wait(&_sem);
	cout << "connecting zookeeper success..." << endl;
}

// zk创建节点
void ZkClient::create(const char *path, const char *data, int datalen, int state)
{
	char path_buffer[128];
	int bufferlen = sizeof(path_buffer);
	// 判断znode节点是否存在，不能在创建
	int flag;
	flag = zoo_exists(_zhandle, path, 0, nullptr);
	if (ZNONODE == flag)
	{
		// 表示znode节点不存在，创建临时节点
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

// get znode节点的值
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

// 发送心跳
void ZkClient::sendHeartBeat()
{
	// 启动线程发送zk心跳，维护session
	thread t([&]() {
		for (;;)
		{
			// 心跳时间设置为timeout时间的1/3
			int time = XmlConfig::getInstance().getTimeout()*1.0 / 3;
			this_thread::sleep_for(chrono::seconds(time));
			zoo_exists(_zhandle, _rootNodePath.c_str(), 0, nullptr);
		}
	});
	t.detach();
}

// 设置存放RPC服务的根节点名称
void ZkClient::setRootPath(string path)
{
	_rootNodePath = path;
}

string ZkClient::getRootPath()
{
	return _rootNodePath;
}

// 这个watcher是全局的回调
void ZkClient::global_watcher(zhandle_t *zh, int type,
	int state, const char *path, void *watcherCtx)
{
	cout << "watcher type:" << type << endl;
	cout << "watcher state:" << state << endl;
	cout << "watcher path:" << path << endl;

	if (type == ZOO_SESSION_EVENT) // session有关的事件
	{
		if (state == ZOO_CONNECTED_STATE) // session创建成功了
		{
			sem_post(&_sem); // 通知调用线程连接成功
		}
		else if (state == ZOO_EXPIRED_SESSION_STATE) // session超时了
		{
			
		}
	}
}