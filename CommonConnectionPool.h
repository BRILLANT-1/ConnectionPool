#pragma once
/*实现连接池模块*/
#include<string.h>
#include<queue>
#include<mutex>
#include<atomic>
#include<iostream>
#include<thread>
#include<condition_variable>
#include<memory>
#include<functional>
using namespace std;
#include"Connection.h"
class Connectionpool
{
public:
	static Connectionpool* getConnectionpool();//获取连接池对象实例
	shared_ptr<Connection> getConnection();//给外部提供接口，从连接池获取一个可用的空闲连接
private:
	Connectionpool();//单例1 
	~Connectionpool();

	bool loadConfigFile();//从配置文件中加载配置项
	//运行在独立的线程中，专门负责产生新连接
	void produceConnectionTask();
	//扫描超过maxIdleTime时间的空闲连接，进行多余的回收
	void scannerConnectionTask();
	string _ip;//MySQL的IP地址
	unsigned _port;//端口
	string _username;//登录用户名
	string _password;//登录密码
	string _dbname;
	int _initSize;//初始连接量
	int _maxSize;//最大连接量
	int _maxIdleTime;//最大空闲时间
	int _connectionTimeout;//超时时间

	queue<Connection*>_connectionQue;//存储mysql的队列
	mutex _queueMutex;//维护连接队列的线程安全互斥锁
	atomic_int _connectionCnt;//记录所创建的connection连接的总数量
	condition_variable cv;//

	std::atomic_bool _stop;
	std::counting_semaphore<1> _sem{0};
};