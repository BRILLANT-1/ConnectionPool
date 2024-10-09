#include"CommonConnectionpool.h"
#include"Public.h"
#include"Connection.h"
#include<iostream>
using namespace std;

//线程安全的懒汉单例函数接口
Connectionpool* Connectionpool::getConnectionpool()
{
	static Connectionpool pool;
	return &pool;
}
//从配置文件中加载配置项
bool Connectionpool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist");
		return false;
	}
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)
		{
			continue;
		}
		//password=123456\n
		int endidx = str.find('\n',idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "password")
		{
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname= value;
		}
		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut")
		{
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;
}

Connectionpool::Connectionpool()
{

	if (!loadConfigFile())
	{
		return;
	}


	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新一下开始的空闲时间
		_connectionQue.push(p);
		_connectionCnt++;
	}
	//启动一个新线程，作为连接的生产者
	thread produce(std::bind(&Connectionpool::produceConnectionTask, this));
	produce.detach();
	//启动一个新线程，扫描超过maxIdleTime时间的空闲连接，进行多余的回收
	thread scanner(std::bind(&Connectionpool::scannerConnectionTask, this));
	scanner.detach();
}
void Connectionpool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			cv.wait(lock); // 
		}

		
		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); //刷新一下开始的空闲时间
			_connectionQue.push(p);
			_connectionCnt++;
		}

		//通知消费者线程，可以连接
		cv.notify_all();
	}
}

//给外部提供接口，从连接池获取一个可用的空闲连接
shared_ptr<Connection>  Connectionpool::getConnection()
{
	unique_lock<mutex>lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if(cv_status::timeout==cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("获取空闲连接超时……获取连接失败");
				return nullptr;
			}
		}
		
	}
	/*shared_ptr智能指针析构时，会直接delete掉connection资源，相当于调用connection析构函数，
	connection资源就被close掉。这里需要重新定义shared_ptr释放资源的方式，把connection归还到queue中*/
	shared_ptr<Connection>sp(_connectionQue.front(),
		[&](Connection* pcon) {
		unique_lock<mutex>lock(_queueMutex);
		pcon->refreshAliveTime(); //刷新一下开始的空闲时间
		_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();//消费完连接后，通知生产者线程检查，如果队列为空，赶紧生产连接，说明此时并发量大
	return sp;
}

//扫描超过maxIdleTime时间的空闲连接，进行多余的回收
void Connectionpool::scannerConnectionTask()
{
	for (;;)
	{
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// 扫描整个队列，释放多余连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection* p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; //调用~connection（）释放连接
			}
			else
			{
				break; // 队头的连接没有超过_maxIdleTime，其他连接肯定没有
			}
		}
	}
}