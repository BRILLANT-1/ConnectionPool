#pragma once
/*ʵ�����ӳ�ģ��*/
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
	static Connectionpool* getConnectionpool();//��ȡ���ӳض���ʵ��
	shared_ptr<Connection> getConnection();//���ⲿ�ṩ�ӿڣ������ӳػ�ȡһ�����õĿ�������
private:
	Connectionpool();//����1 
	~Connectionpool();

	bool loadConfigFile();//�������ļ��м���������
	//�����ڶ������߳��У�ר�Ÿ������������
	void produceConnectionTask();
	//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж���Ļ���
	void scannerConnectionTask();
	string _ip;//MySQL��IP��ַ
	unsigned _port;//�˿�
	string _username;//��¼�û���
	string _password;//��¼����
	string _dbname;
	int _initSize;//��ʼ������
	int _maxSize;//���������
	int _maxIdleTime;//������ʱ��
	int _connectionTimeout;//��ʱʱ��

	queue<Connection*>_connectionQue;//�洢mysql�Ķ���
	mutex _queueMutex;//ά�����Ӷ��е��̰߳�ȫ������
	atomic_int _connectionCnt;//��¼��������connection���ӵ�������
	condition_variable cv;//

	std::atomic_bool _stop;
	std::counting_semaphore<1> _sem{0};
};