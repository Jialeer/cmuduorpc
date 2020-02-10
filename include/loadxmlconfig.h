#ifndef LOADXMLCONFIG
#define LOADXMLCONFIG

#include <string>
#include <iostream>
using namespace std;

// 默认从rpc_config.xml中加载配置项
class XmlConfig
{
public:
	static XmlConfig& getInstance();
	string getZookeeperHost();
	string getRpcServerIp();
	unsigned short getRpcServerPort();
	int getTimeout();
private:
	XmlConfig();
	XmlConfig(const XmlConfig&) = delete;
	XmlConfig& operator=(const XmlConfig&) = delete;
	string _zkhost;
	string _rpcip;
	unsigned short _rpcport;
	int _timeout;
};

#endif