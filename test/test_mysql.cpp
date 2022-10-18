#include <unistd.h>
#include <fstream>
#include "../include/mysql.h"
#include "../include/log.h"

bool blobToFile();
bool fileToBlob();
bool deleteTable();
bool selectTable();
bool updateTable();
bool insertTable();

static server::Logger::ptr g_logger = SERVER_LOG_NAME("mysql");

int main(int argc, char const *argv[])
{
	std::map<std::string, std::string> params;
	params["host"] = "127.0.0.1";
	params["user"] = "yc";
	params["passwd"] = "436052";
	params["dbname"] = "testmysql";

	server::MySQL::ptr mysql(new server::MySQL(params));
	if(!mysql->connect())
	{
		std::cout << "connect fail" << std::endl;
	}

	// int id = 0;
	// std::string name = "test";
	// double weight = 37.58;

	// server::MySQLStmt::ptr stmt = server::MySQLStmt::Create(mysql, 
	// 	"insert into girls(id,name,weight,btime,pic) values(?,?,?,?,?)");

	// // 模拟超女数据，向表中插入5条测试数据
	// for (int i = 6; i < 11; i++)
	// {
	// 	id = i + 1;
	// 	name = "西施" + std::to_string(i) + "frank";
	// 	weight = 45.55 + i;

	// 	std::ostringstream pic;
	// 	std::string filename = "./data/img/" + std::to_string(i - 5) + ".jpg";
	// 	std::fstream file;
	// 	file.open(filename, std::ios::in);

	// 	if (!file.is_open())
	// 	{
	// 		SERVER_LOG_INFO(g_logger) << "open file failed!";
	// 		return 0;
	// 	}
	// 	char ch;
	// 	while (pic && file.get(ch))
	// 	{
	// 		pic.put(ch);
	// 	}

	// 	stmt->bind(1, id);
	// 	stmt->bind(2, name);
	// 	stmt->bind(3, weight);
	// 	stmt->bindTime(4, time(0));
	// 	stmt->bindBlob(5, pic.str());

	// 	if (stmt->execute() != 0)
	// 	{
	// 		SERVER_LOG_ERROR(g_logger) << stmt->getErrStr();
	// 		return false;
	// 	}
	// }

	auto res = std::dynamic_pointer_cast<server::MySQLRes>(
		mysql->query("select id,name,weight,date_format(btime,'%%Y-%%m-%%d %%H:%%i:%%s') from girls where id in (2,3,4)"));

	// SERVER_LOG_INFO(g_logger) << res->isNull(1);
	if(!res)
	{
		SERVER_LOG_ERROR(g_logger) << "invalid";
		return false;
	}

	if(res->getErrno()) {
		SERVER_LOG_ERROR(g_logger) << "errno=" << res->getErrno()
								   << " errstr=" << res->getErrStr() << std::endl;
		return false;
	}
	
	while (res->next())
	{
		int64_t girl_id = res->getInt64(0);
		std::string name = res->getString(1);
		SERVER_LOG_INFO(g_logger) << girl_id << " - " << name;
	}
	return 0;
}

bool blobToFile()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	struct st_girls
	{
		long id;
		char pic[100000];
		unsigned long picsize;
	} stgirls;

	SqlStatement stmt(conn);
	
	stmt.prepare("select id,pic from girls where id in (2,3,4)");
	stmt.bindout(1, &stgirls.id);
	stmt.bindoutlob(2, stgirls.pic, sizeof(stgirls.pic),&stgirls.picsize);

	if (stmt.execute() != 0)
	{
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
		return false;
	}

	char filename[1024];
	while(true)
	{
		memset(&stgirls, 0, sizeof(stgirls));

		if (stmt.next() != 0)
			break;
		
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "../data/img/%02ld_out.jpg", stgirls.id);
		buftofile(filename, stgirls.pic, stgirls.picsize);
	}

	conn->commit();

	return 0;
}

bool fileToBlob()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	struct st_girls
	{
		long id;
		char pic[100000];
		unsigned long picsize;
	} stgirls;

	SqlStatement stmt(conn);

	stmt.prepare("update girls set pic=:1 where id=:2");
	stmt.bindinlob(1, stgirls.pic, &stgirls.picsize);
	stmt.bindin(2, &stgirls.id);

	for (int i = 1; i < 3; i++)
	{
		memset(&stgirls, 0, sizeof(struct st_girls));

		stgirls.id = i;
		if (i == 1)
			stgirls.picsize = filetobuf("../data/img/1.jpg", stgirls.pic);
		if (i == 2)
			stgirls.picsize = filetobuf("../data/img/2.jpg", stgirls.pic);
		
		if (stmt.execute() != 0)
		{
			printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
			return false;
		}

		printf("成功更新了%ld条记录, %s。\n", stmt.m_cda.rpc, stmt.m_sql);
	}

	printf("update table girls ok.\n");

	conn->commit();

	return 0;
}

bool deleteTable()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	// 操作SQL语句的对象
	SqlStatement stmt(conn);
	int iminid, imaxid;

	// 准备更新数据的SQL语句
	// 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic
	stmt.prepare("delete from girls where id>=:1 and id<=:2");
	stmt.bindin(1, &iminid);
	stmt.bindin(2, &imaxid);

	iminid = 1;
	imaxid = 3;

	if (stmt.execute() != 0)
	{
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
		return false;
	}

	printf("本次删除了%ld条记录。\n", stmt.m_cda.rpc);

	conn->commit();

	return 0;
}

bool selectTable()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	struct st_girls
	{
		long id;
		char name[31];
		double weight;
		char btime[20];
	} stgirls;

	// 操作SQL语句的对象
	SqlStatement stmt(conn);

	// 准备更新数据的SQL语句
	// 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic
	stmt.prepare(
		"select id,name,weight,date_format(btime,'%%Y-%%m-%%d %%H:%%i:%%s') from girls where id in (2,3,4)");
	stmt.bindout(1, &stgirls.id);
	stmt.bindout(2, stgirls.name, 30);
	stmt.bindout(3, &stgirls.weight);
	stmt.bindout(4, stgirls.btime, 19);
	
	if (stmt.execute() != 0)
	{
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
		return false;
	}

	while (true)
	{
		memset(&stgirls, 0, sizeof(struct st_girls));

		if (stmt.next() != 0)
			break;
		
		// 把获取到的记录的值打印出来
		printf("id=%ld, name=%s, weight=%.02f, btime=%s\n",
			   stgirls.id, stgirls.name, stgirls.weight, stgirls.btime);
	}

	conn->commit();

	return true;
}

bool updateTable()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	struct st_girls
	{
		long id;
		char name[31];
		double weight;
		char btime[20];
	} stgirls;

	// 操作SQL语句的对象
	SqlStatement stmt(conn);

	// 准备更新数据的SQL语句
	// 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic
	stmt.prepare(
		"update girls set name=:1,weight=:2,btime=str_to_date(:3,'%%Y-%%m-%%d %%H:%%i:%%s') where id=:4");
	stmt.bindin(1, stgirls.name, 30);
	stmt.bindin(2, &stgirls.weight);
	stmt.bindin(3, stgirls.btime, 19);
	stmt.bindin(4, &stgirls.id);

	// 模拟超女数据，向表中插入5条测试数据
	for (int i = 0; i < 5; i++)
	{
		memset(&stgirls, 0, sizeof(struct st_girls));

		stgirls.id = i + 1;
		sprintf(stgirls.name, "毛晓彤%05dSakura", i + 1);
		stgirls.weight = 50.55 + i;
		sprintf(stgirls.btime, "2022-9-9 15:31:%02d", i);

		if (stmt.execute() != 0)
		{
			printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
			return false;
		}

		printf("成功更新了%ld条记录, %s。\n", stmt.m_cda.rpc, stmt.m_sql);
	}

	// 提交事务
	conn->commit();

	return true;
}

bool insertTable()
{
	// 数据库连接类
	Connection::ptr conn;

	// 登录数据库，返回值：0-成功；其它是失败，存放了MySQL的错误代码
	// 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
	if (conn->connecttodb("127.0.0.1,yc,436052,testmysql,3306", "utf8")!=0) {
		printf("connect database failed.\n%s\n", conn->m_cda.message); 
		return false;
	}

	struct st_girls
	{
		long id;
		char name[31];
		double weight;
		char btime[20];
	} stgirls;

	// 操作SQL语句的对象
	SqlStatement stmt(conn);

	// 准备插入数据的SQL语句
	// 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic
	stmt.prepare(
		"insert into girls(id,name,weight,btime) values(:1,:2,:3,str_to_date(:4,'%%Y-%%m-%%d %%H:%%i:%%s'))");
	stmt.bindin(1, &stgirls.id);
	stmt.bindin(2, stgirls.name, 30);
	stmt.bindin(3, &stgirls.weight);
	stmt.bindin(4, stgirls.btime, 19);

	// 模拟超女数据，向表中插入5条测试数据
	for (int i = 1; i < 3; i++)
	{
		memset(&stgirls, 0, sizeof(struct st_girls));

		stgirls.id = i + 1;
		sprintf(stgirls.name, "鞠婧祎%05dSakura", i + 1);
		stgirls.weight = 45.55 + i;
		sprintf(stgirls.btime, "2022-9-9 10:33:%02d", i);

		if (stmt.execute() != 0)
		{
			printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
			return false;
		}

		printf("成功插入了%ld条记录, %s。\n", stmt.m_cda.rpc, stmt.m_sql);
	}

	// 提交事务
	conn->commit();

	return true;
}