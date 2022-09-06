#include "_public.h"
#include "_ftp.h"

struct st_arg
{
	char 	host[31];			// 远程服务器的IP和端口
	int 	mode;				// 传输模式，1-被动模式，2-主动模式，缺省采用被动模式
	char 	username[31];		// 远程服务器ftp的用户名。
	char	password[31];		// 远程服务器ftp的密码。
	char	remotepath[301];	// 远程服务器存放文件的目录
	char	localpath[301];		// 本地文件存放的目录。
	char	matchname[101];		// 待下载文件匹配的规则
	char 	listfilename[301];	// 下载前列出服务器文件名的文件
} starg;

struct st_fileinfo
{
	char filename[301];
	char mtime[21];
};

vector<struct st_fileinfo> vfileinfo;

bool LoadListFile();

CLogFile logfile;

Cftp ftp;

bool _xmltoarg(const char *strxmlbuffer);

void _help();

bool _ftpgetfiles();

// 程序退出和信号2、15的处理函数
void EXIT(int sig);

int main(int argc, char const *argv[])
{
	// 小目标, 把ftp服务器上某目录中的文件下载到本地的目录中。

	if (argc != 3) { _help(); return -1; }

	// 处理程序的退出信号
	// 关闭全部的信号和输入输出
	// CloseIOAndSignal(true); 
	signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

	// 打开日志文件
	if (logfile.Open(argv[1], "a+", true) == false) {
        printf("ftpgetfiles logfile.Open(%s) failed.\n", argv[3]); 
        return -1;
    } 

	// 解析xml，得到程序运行的参数
	if (_xmltoarg(argv[2]) == false) return -1;

	// 登录ftp服务器
	if (ftp.login(starg.host, starg.username, starg.password, starg.mode) == false)
	{
		logfile.Write("ftp.login (%s) failed.\n", starg.host); return -1;
	}
	logfile.Write("ftp.login (%s) ok.\n", starg.host);

	_ftpgetfiles();

	ftp.logout();

	return 0;
}

bool _ftpgetfiles()
{
	// 进入ftp服务器存放文件的目录
	if (ftp.chdir(starg.remotepath) == false)
	{
		printf("ftp.chdir %s failed.\n", starg.remotepath); return false;
	}
	logfile.Write("ftp.chdir (%s) ok.\n", starg.remotepath);

	// 调用ftp.nlist()方法列出服务器目录中的文件，结果存放到本地文件中
	if (ftp.nlist(".", starg.listfilename) == false)
	{
		logfile.Write("ftp.nlist (%s) failed.\n", starg.listfilename); return false;
	}

	// 把ftp.nlist()方法获取到的list文件加载到容器vfilelist中
	if (LoadListFile() == false)
	{
		logfile.Write("LoadListFile failed.\n"); return false;
	}
	char strremotefilename[301], strlocalfilename[301];
	// 遍历容器vfilelist
	for (int ii=0; ii < vfileinfo.size(); ii++)
	{
		logfile.Write("filename %s.\n", vfileinfo[ii].filename);
		SNPRINTF(strremotefilename, sizeof(strremotefilename), 300, "%s/%s", starg.remotepath, vfileinfo[ii].filename);
		SNPRINTF(strlocalfilename, sizeof(strlocalfilename), 300, "%s/%s", starg.localpath, vfileinfo[ii].filename);
		// 调用ftp.get()方法从服务器下载文件。
		logfile.Write("get %s ...", strremotefilename);
		
		if (ftp.get(strremotefilename, strlocalfilename) == false)
		{
			logfile.WriteEx("failed.\n"); break;
		}
		logfile.WriteEx("ok.\n");
	}

	return true;
}

bool LoadListFile()
{
	vfileinfo.clear();

	CFile File;

	if (File.Open(starg.listfilename, "r") == false)
	{
		logfile.Write("File.Open(%s) failed.\n", starg.listfilename); return false;
	}

	struct st_fileinfo stfileinfo;

	while (true)
	{
		memset(&stfileinfo, 0, sizeof(struct st_fileinfo));

		if (File.Fgets(stfileinfo.filename, 300, true) == false) break;

		if (MatchStr(stfileinfo.filename, starg.matchname) == false) continue;

		vfileinfo.push_back(stfileinfo);
	}

	return true;
}

void EXIT(int sig)
{
	printf("程序退出, sig=%d\n\n", sig);

	exit(0);
}

void _help()
{
	printf("\n");
	printf("Using:/project/tools/bin/ftpgetfiles logfilename xmlbuffer\n\n");
	printf("Sample:/project/tools/bin/procctl 30 /project/tools/bin/ftpgetfiles /project/logfile/ftpgetfiles_surfdata.log " 	\
			"\"<host>127.0.0.1:21</host>"																			\
			"<mode>1</mode>"                                                                                         \
			"<username>yc</username>"																			\
			"<password>436052</password>"																			\
			"<localpath>/project/tmp/local/surfdata</localpath>"																\
			"<remotepath>/project/tmp/idc/surfdata</remotepath>" 															\
			"<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>" 														\
			"<listfilename>/project/demo/test/ftpgetfiles_surfdata.list</listfilename>" 								\
			"<ptype>3</ptype>"																						\
			"<remotepathbak>/tmp/idc/surfdatabak</remotepathbak>" 													\
			"<okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename>\"\n\n");

	printf("本程序是通用的功能模块, 用于把远程ftp服务器的文件下载到本地目录。\n");
	printf("logfilename是本程序运行的日志文件。\n");
	printf("xmlbuffer为文件下载的参数, 如下：\n");
	printf("<host>127.0.0.1:21</host> 远程服务器的IP和端口。\n");
	printf("<mode>1</mode> 传输模式, 1-被动模式, 2-主动模式，缺省采用被动模式。\n");
	printf("<username>wydxry</username> 远程服务器ftp的用户名。\n");
	printf("<password>wydxrypwd</password> 远程服务器ftp的密码。\n");
	printf("<remotepath>/project/tmp/idc/surfdata</remotepath> 远程服务器存放文件的目录。\n");
	printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录。\n");
	printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待下载文件匹配的规则。"	\
			"不匹配的文件不会被下载，本字段尽可能设置精确，不建议用*匹配全部的文件。\n");
	printf("<listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename> 下载前列出服务器文件名的文件。\n");
	printf("<ptype>1</ptype> 文件下载成功后, 远程服务器文件的处理方式: 1-什么也不做; 2-删除; 3-备份, 如果为3, 还要指定备份的目录。\n");
	printf("<remotepathbak>/tmp/idc/surfdatabak</remotepathbak> 文件下载成功后, 服务器文件的备份目录, 此参数只有当ptype=3时才有效。\n");
	printf("<okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename> 已下载成功文件名清单, 此参数只有当ptype=1时才有效。\n");
	printf("<checkmtime>true</checkmtime> 是否需要检查服务端文件的时间, true-需要, false-不需要, 此参数只有当ptype=1时才有效, 缺省为false。\n");
	printf("<timeout>80</timeout> 下载文件超时时间，单位：秒，视文件大小和网络带宽而定。\n");
	printf("<pname>ftpgetfiles_surfdata</pname> 进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");
}

bool _xmltoarg(const char *strxmlbuffer)
{
	// 解析xml，得到程序运行的参数
	memset(&starg, 0, sizeof(struct st_arg));

	GetXMLBuffer(strxmlbuffer, "host", starg.host, 30);			// 远程服务器的IP和端口
	if (strlen(starg.host) == 0)
	{ logfile.Write("host is null.\n"); return -1; }
	
	GetXMLBuffer(strxmlbuffer, "mode", &starg.mode);				// 传输模式，1-被动模式，2-主动模式，缺省采用被动模式
	if (starg.mode != 2) starg.mode = 1;

	GetXMLBuffer(strxmlbuffer, "username", starg.username, 30);	// 远程服务器ftp的用户名
	if (strlen(starg.username) == 0)
	{ logfile.Write("username is null.\n"); return -1; }

	GetXMLBuffer(strxmlbuffer, "password", starg.password, 30);	// 远程服务器ftp的密码。
	if (strlen(starg.password) == 0)
	{ logfile.Write("password is null.\n"); return -1; }

	GetXMLBuffer(strxmlbuffer, "remotepath", starg.remotepath, 300);	// 远程服务器存放文件的目录
	if (strlen(starg.remotepath) == 0)
	{ logfile.Write("remotepath is null.\n"); return -1; }

	GetXMLBuffer(strxmlbuffer, "localpath", starg.localpath, 300);	// 本地文件存放的目录。
	if (strlen(starg.localpath) == 0)
	{ logfile.Write("localpath is null.\n"); return -1; }

	GetXMLBuffer(strxmlbuffer, "listfilename", starg.listfilename, 301);	// 记录服务器目录的本地文件。
	if (strlen(starg.listfilename) == 0)
	{ logfile.Write("listfilename is null.\n"); return -1; }

	GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 100);	// 本地文件存放的目录。
	if (strlen(starg.matchname) == 0)
	{ logfile.Write("matchname is null.\n"); return -1; }

	return true;
}