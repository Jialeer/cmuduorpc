#include "loadxmlconfig.h"
#include "tinyxml.h"

XmlConfig& XmlConfig::getInstance()
{
	static XmlConfig xml;
	return xml;
}
string XmlConfig::getZookeeperHost()
{
	return _zkhost;
}
string XmlConfig::getRpcServerIp()
{
	return _rpcip;
}
unsigned short XmlConfig::getRpcServerPort()
{
	return _rpcport;
}
int XmlConfig::getTimeout()
{
	return _timeout;
}
XmlConfig::XmlConfig()
{
	TiXmlDocument lconfigXML;
	if (!lconfigXML.LoadFile("rpc_cfg.xml"))
	{
		cout << "load zookeeper config error...check rpc_cfg.xml is right?" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		const TiXmlNode *node = lconfigXML.RootElement()->IterateChildren("RpcServer", nullptr);
		const TiXmlNode *node2 = nullptr;
		// rpc client没有RpcServer配置项
		if (node != nullptr)
		{
			node2 = node->IterateChildren("ip", nullptr);
			_rpcip = node2->ToElement()->GetText();
			node2 = node->IterateChildren("port", nullptr);
			_rpcport = atoi(node2->ToElement()->GetText());
		}

		// rpc server和rpc client都有zookeeper配置项
		node = lconfigXML.RootElement()->IterateChildren("zookeeper", nullptr);
		node2 = node->IterateChildren("host", nullptr);
		_zkhost = node2->ToElement()->GetText();
		node2 = node->IterateChildren("timeout", nullptr);
		_timeout = atoi(node2->ToElement()->GetText());

		cout << "load rpc_cfg.xml success..." << endl;
	}
}