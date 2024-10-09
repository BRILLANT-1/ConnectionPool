
#include<iostream>
#include<mysql.h>
using namespace std;
#include"Connection.h"
#include"CommonConnectionPool.h"
int main()
{
	Connection conn;
	conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
	/*Connection conn;
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
		"zhang san", 20, "male");
	conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
	conn.update(sql);
	return 0;*/

	//Connectionpool* cp = Connectionpool::getConnectionpool();
	//cp->loadConfigFile();
	clock_t begin = clock();
	//Connectionpool* cp = Connectionpool::getConnectionpool();
	thread t1([]()
		{
			//Connectionpool* cp = Connectionpool::getConnectionpool();
			for (int i = 0; i < 250; ++i)
			{
				/*char sql[1024] = {0};
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				shared_ptr<Connection>sp = cp->getConnection();
				sp->update(sql);*/
				Connection conn;
				char sql[1024] = { 0 };
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
				conn.update(sql);

			}
		});
	thread t2([]()
		{
			//Connectionpool* cp = Connectionpool::getConnectionpool();
			for (int i = 0; i < 250; ++i)
			{
				/*char sql[1024] = {0};
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				shared_ptr<Connection>sp = cp->getConnection();
				sp->update(sql);*/
				Connection conn;
				char sql[1024] = { 0 };
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
				conn.update(sql);
			}
		});
	thread t3([]()
		{
			//Connectionpool* cp = Connectionpool::getConnectionpool();
			for (int i = 0; i < 250; ++i)
			{
				/*char sql[1024] = {0};
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				shared_ptr<Connection>sp = cp->getConnection();
				sp->update(sql);*/
				Connection conn;
				char sql[1024] = { 0 };
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
				conn.update(sql);

			}
		
		});
	thread t4([]()
		{
			//Connectionpool* cp = Connectionpool::getConnectionpool();
			for (int i = 0; i < 250; ++i)
			{
				/*char sql[1024] = {0};
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				shared_ptr<Connection>sp = cp->getConnection();
				sp->update(sql);*/
				Connection conn;
				char sql[1024] = { 0 };
				sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
					"zhang san", 20, "male");
				conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
				conn.update(sql);

			}
		});
	t1.join();t2.join(); t3.join(); t4.join();
	
	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
	return 0;
#if 0
	for (int i = 0; i < 10000; ++i)
	{
		/*Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
		conn.update(sql);*/
		//使用连接池
		shared_ptr<Connection>sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		sp->update(sql);
	}
#endif
}