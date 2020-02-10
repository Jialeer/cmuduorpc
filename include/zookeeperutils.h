#ifndef ZOOKEEPERUTILS_H
#define ZOOKEEPERUTILS_H

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class ZkClient
{
public:
	ZkClient();
	
	void start();

	// zk�����ڵ�
	void create(const char *path, const char *data, int datalen, int state=0);
	
	// get znode�ڵ��ֵ
	string get(const char *path);
	
	// ��������
	void sendHeartBeat();

	// ���ô��RPC����ĸ��ڵ�����
	static void setRootPath(string path);

	static string getRootPath();

	// ���watcher��ȫ�ֵĻص�
	static void global_watcher(zhandle_t *zh, int type,
		int state, const char *path, void *watcherCtx);

private:
	// zkclient��zkserverͨ���õľ��
	zhandle_t *_zhandle;
	// ͬ��session�����ɹ�
	static sem_t _sem;
	// ����zk�ϴ��RPC����ĸ��ڵ����ƣ�������д��/RpcService�����
	static string _rootNodePath;
};

#endif