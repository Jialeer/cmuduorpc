# cmuduo-rpc

C++语言编写开发的一个RPC调用框架，网络服务器基于muduo库实现，RPC服务器和客户端通讯的数据系列化和反序列化使用protobuf，服务配置中心采用zookeeper，配置文件加载采用tinyxml

# 环境

需要安装protobuf、zookeeper以及muduo网络库环境

# 项目文件介绍

/bin：test文件夹测试代码生成的可执行文件 以及配置文件
/lib：rpc框架生成的静态库，可以直接链接使用  
/build：cmake编译生成的中间文件  
/src：rpc框架源代码  
/test：rpc框架测试代码  
/thirdparty：第三方代码，这里主要用了tinyxml  

# 编译

项目采用cmake编译，可以在项目根目录执行下面一键编译  

​			`chmod 700 autobuild.sh  //给当前用户赋予执行权限`

​			`./autobuild.sh`  

# 运行

1.先启动zookeeper   

​			 `cd /usr/local/zookeeper/zookeeper-3.4.10/bin/`

​			`su         //进入root`

​			`******//输入密码`

​			`./zkServer.sh start`		

2.创建RPC服务节点

​			`cd /usr/local/zookeeper/zookeeper-3.4.10/bin/`

​			`./zkCli.sh`

​			`...`

​			`ls /`

​			`create /RpcService data`

3.启动testserver，注意同级目录下需要加载配置文件rpc_cfg.xml，里面主要包含RpcServer和zookeeper的数据配置  

​			`./testserver`

4.启动testclient，注意同级目录下需要加载配置文件rpc_cfg.xml，里面主要包含zookeeper的数据配置  

​			`./testclient`

