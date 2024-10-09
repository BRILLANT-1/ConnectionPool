#include"CommonConnectionpool.h"
#include"Public.h"
#include"Connection.h"
#include<iostream>
using namespace std;

//�̰߳�ȫ���������������ӿ�
Connectionpool* Connectionpool::getConnectionpool()
{
	static Connectionpool pool;
	return &pool;
}
//�������ļ��м���������
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
		p->refreshAliveTime();//ˢ��һ�¿�ʼ�Ŀ���ʱ��
		_connectionQue.push(p);
		_connectionCnt++;
	}
	//����һ�����̣߳���Ϊ���ӵ�������
	thread produce(std::bind(&Connectionpool::produceConnectionTask, this));
	produce.detach();
	//����һ�����̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж���Ļ���
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
			p->refreshAliveTime(); //ˢ��һ�¿�ʼ�Ŀ���ʱ��
			_connectionQue.push(p);
			_connectionCnt++;
		}

		//֪ͨ�������̣߳���������
		cv.notify_all();
	}
}

//���ⲿ�ṩ�ӿڣ������ӳػ�ȡһ�����õĿ�������
shared_ptr<Connection>  Connectionpool::getConnection()
{
	unique_lock<mutex>lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if(cv_status::timeout==cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("��ȡ�������ӳ�ʱ������ȡ����ʧ��");
				return nullptr;
			}
		}
		
	}
	/*shared_ptr����ָ������ʱ����ֱ��delete��connection��Դ���൱�ڵ���connection����������
	connection��Դ�ͱ�close����������Ҫ���¶���shared_ptr�ͷ���Դ�ķ�ʽ����connection�黹��queue��*/
	shared_ptr<Connection>sp(_connectionQue.front(),
		[&](Connection* pcon) {
		unique_lock<mutex>lock(_queueMutex);
		pcon->refreshAliveTime(); //ˢ��һ�¿�ʼ�Ŀ���ʱ��
		_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();//���������Ӻ�֪ͨ�������̼߳�飬�������Ϊ�գ��Ͻ��������ӣ�˵����ʱ��������
	return sp;
}

//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж���Ļ���
void Connectionpool::scannerConnectionTask()
{
	for (;;)
	{
		// ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// ɨ���������У��ͷŶ�������
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection* p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; //����~connection�����ͷ�����
			}
			else
			{
				break; // ��ͷ������û�г���_maxIdleTime���������ӿ϶�û��
			}
		}
	}
}