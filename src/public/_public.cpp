/*
 * @FilePath: /project/public/_public.cpp
 * @Description: 此程序是开发框架公用函数和类的定义文件。
 * @Autor: yc
 * @Date: 2022-07-09 00:35:11
 * @LastEditors: yc
 * @LastEditTime: 2022-07-21 16:18:23
 */

#include "_public.h"

char *STRCPY(char* dest, const size_t destlen, const char* src)
{
	if (dest==0) return 0;		// 判断空指针
	memset(dest, 0, destlen);	// 初始化dest
	// memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。
	if (src==0) return dest;

	if (strlen(src) > destlen-1) strncpy(dest, src, destlen-1);
	else strcpy(dest, src);

	return dest;
}

char *STRNCPY(char* dest,const size_t destlen,const char* src,size_t n)
{
	if (dest==0) return 0;    // 判断空指针。
	memset(dest,0,destlen);   // 初始化dest。
	// memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。
	if (src==0) return dest;

	if (n>destlen-1) strncpy(dest,src,destlen-1); 
	else strncpy(dest,src,n);

	return dest;
}

char *STRCAT(char* dest,const size_t destlen,const char* src)
{
	if (dest==0) return 0;    // 判断空指针。
	if (src==0) return dest;

	unsigned int left=destlen-1-strlen(dest);

	if (strlen(src)>left) { strncat(dest,src,left); dest[destlen-1]=0; }
	else strcat(dest,src);

	return dest;
}

char *STRNCAT(char* dest,const size_t destlen,const char* src,size_t n)
{
	if (dest==0) return 0;    // 判断空指针。
	if (src==0) return dest;

	size_t left=destlen-1-strlen(dest);

	if (n>left) { strncat(dest,src,left); dest[destlen-1]=0; }
	else strncat(dest,src,n);

	return dest;
}

int SPRINTF(char *dest,const size_t destlen,const char *fmt,...)
{
	if (dest==0) return -1;    // 判断空指针。

	memset(dest,0,destlen);
	// memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。

	va_list arg;
	va_start(arg,fmt);
	int ret=vsnprintf(dest,destlen,fmt,arg );
	va_end(arg);

	return ret;
}

int SNPRINTF(char *dest,const size_t destlen,size_t n,const char *fmt,...)
{
	if (dest==0) return -1;    // 判断空指针。

	memset(dest,0,destlen);
	// memset(dest,0,sizeof(dest));   // 不能这么写，在64位系统中，sizeof(dest)永远是8。

	int len=n;
	if (n>destlen) len=destlen;

	va_list arg;
	va_start(arg,fmt);
	int ret=vsnprintf(dest,len,fmt,arg );
	va_end(arg);

	return ret;
}

void DeleteLChar(char *str,const char chr)
{
	if (str == 0) return;
	if (strlen(str) == 0) return;

	char strTemp[strlen(str)+1];

	int iTemp=0;

	memset(strTemp,0,sizeof(strTemp));
	strcpy(strTemp,str);

	while ( strTemp[iTemp] == chr )  iTemp++;

	memset(str,0,strlen(str)+1);

	strcpy(str,strTemp+iTemp);

	return;
}

void DeleteRChar(char *str,const char chr)
{
	if (str == 0) return;
	if (strlen(str) == 0) return;

	int istrlen = strlen(str);

	while (istrlen>0)
	{
		if (str[istrlen-1] != chr) break;

		str[istrlen-1]=0;

		istrlen--;
	}
}

void DeleteLRChar(char *str,const char chr)
{
	DeleteLChar(str,chr);
	DeleteRChar(str,chr);
}

void ToUpper(char *str)
{
	if (str == 0) return;

	if (strlen(str) == 0) return;

	int istrlen=strlen(str);

	for (int ii=0;ii<istrlen;ii++)
	{
		if ( (str[ii] >= 'a') && (str[ii] <= 'z') ) str[ii]=str[ii] - 32;
	}
}

void ToUpper(string &str)
{
	if (str.empty()) return;

	char strtemp[str.size()+1];

	STRCPY(strtemp,sizeof(strtemp),str.c_str());

	ToUpper(strtemp);

	str=strtemp;

	return;
}

void ToLower(char *str)
{
	if (str == 0) return;

	if (strlen(str) == 0) return;

	int istrlen=strlen(str);

	for (int ii=0;ii<istrlen;ii++)
	{
		if ( (str[ii] >= 'A') && (str[ii] <= 'Z') ) str[ii]=str[ii] + 32;
	}
}

void ToLower(string &str)
{
	if (str.empty()) return;

	char strtemp[str.size()+1];

	STRCPY(strtemp,sizeof(strtemp),str.c_str());

	ToLower(strtemp);

	str=strtemp;

	return;
}

void UpdateStr(char *str,const char *str1,const char *str2,bool bloop)
{
	if (str == 0) return;
	if (strlen(str) == 0) return;
	if ( (str1 == 0) || (str2 == 0) ) return;

	// 如果bloop为true并且str2中包函了str1的内容，直接返回，因为会进入死循环，最终导致内存溢出。
	if ( (bloop==true) && (strstr(str2,str1)>0) ) return;

	// 尽可能分配更多的空间，但仍有可能出现内存溢出的情况，最好优化成string。
	int ilen=strlen(str)*10;
	if (ilen<1000) ilen=1000;

	char strTemp[ilen];

	char *strStart=str;

	char *strPos=0;

	while (true)
	{
		if (bloop == true)
		{
		strPos=strstr(str,str1);
		}
		else
		{
		strPos=strstr(strStart,str1);
		}

		if (strPos == 0) break;

		memset(strTemp,0,sizeof(strTemp));
		STRNCPY(strTemp,sizeof(strTemp),str,strPos-str);
		STRCAT(strTemp,sizeof(strTemp),str2);
		STRCAT(strTemp,sizeof(strTemp),strPos+strlen(str1));
		strcpy(str,strTemp);

		strStart=strPos+strlen(str2);
	}
}

void PickNumber(const char *src,char *dest,const bool bsigned,const bool bdot)
{
	if (dest==0) return;    // 判断空指针。
	if (src==0) { strcpy(dest,""); return; }

	char strtemp[strlen(src)+1];
	memset(strtemp,0,sizeof(strtemp));
	strcpy(strtemp,src);
	DeleteLRChar(strtemp,' ');

	int ipossrc,iposdst,ilen;
	ipossrc=iposdst=ilen=0;

	ilen=strlen(strtemp);

	for (ipossrc=0;ipossrc<ilen;ipossrc++)
	{
		if ( (bsigned==true) && (strtemp[ipossrc] == '+') )
		{
		dest[iposdst++]=strtemp[ipossrc]; continue;
		}

		if ( (bsigned==true) && (strtemp[ipossrc] == '-') )
		{
		dest[iposdst++]=strtemp[ipossrc]; continue;
		}

		if ( (bdot==true) && (strtemp[ipossrc] == '.') )
		{
		dest[iposdst++]=strtemp[ipossrc]; continue;
		}

		if (isdigit(strtemp[ipossrc])) dest[iposdst++]=strtemp[ipossrc];
	}

	dest[iposdst]=0;
}

bool MatchStr(const string &str,const string &rules)
{
	// 如果用于比较的字符是空的，返回false
	if (rules.size() == 0) return false;

	// 如果被比较的字符串是"*"，返回true
	if (rules == "*") return true;

	int ii,jj;
	int  iPOS1,iPOS2;
	CCmdStr CmdStr,CmdSubStr;

	string strFileName,strMatchStr;

	strFileName=str;
	strMatchStr=rules;

	// 把字符串都转换成大写后再来比较
	ToUpper(strFileName);
	ToUpper(strMatchStr);

	CmdStr.SplitToCmd(strMatchStr,",");

	for (ii=0;ii<CmdStr.CmdCount();ii++)
	{
		// 如果为空，就一定要跳过，否则就会被配上
		if (CmdStr.m_vCmdStr[ii].empty() == true) continue;

		iPOS1=iPOS2=0;
		CmdSubStr.SplitToCmd(CmdStr.m_vCmdStr[ii],"*");

		for (jj=0;jj<CmdSubStr.CmdCount();jj++)
		{
		// 如果是文件名的首部
		if (jj == 0)
		{
			if (strncmp(strFileName.c_str(),CmdSubStr.m_vCmdStr[jj].c_str(),CmdSubStr.m_vCmdStr[jj].size()) != 0) break;
		}

		// 如果是文件名的尾部
		if (jj == CmdSubStr.CmdCount()-1)
		{
			if (strcmp(strFileName.c_str()+strFileName.size()-CmdSubStr.m_vCmdStr[jj].size(),CmdSubStr.m_vCmdStr[jj].c_str()) != 0) break;
		}

		iPOS2=strFileName.find(CmdSubStr.m_vCmdStr[jj],iPOS1);

		if (iPOS2 < 0) break;

		iPOS1=iPOS2+CmdSubStr.m_vCmdStr[jj].size();
		}

		if (jj==CmdSubStr.CmdCount()) return true;
	}

	return false;
}

// 根据绝对路径的文件名或目录名逐级的创建目录。
// pathorfilename：绝对路径的文件名或目录名。
// bisfilename：说明pathorfilename的类型，true-pathorfilename是文件名，否则是目录名，缺省值为true。
// 返回值：true-创建成功，false-创建失败，如果返回失败，原因有大概有三种情况：1）权限不足； 2）pathorfilename参数不是合法的文件名或目录名；3）磁盘空间不足。
bool MKDIR(const char *filename,bool bisfilename)
{
  // 检查目录是否存在，如果不存在，逐级创建子目录
  char strPathName[301];

  int ilen=strlen(filename);

  for (int ii=1; ii<ilen;ii++)
  {
    if (filename[ii] != '/') continue;

    STRNCPY(strPathName,sizeof(strPathName),filename,ii);

    if (access(strPathName,F_OK) == 0) continue; // 如果目录已存在，continue

    if (mkdir(strPathName,0755) != 0) return false;  // 如果目录不存在，创建它。
  }

  if (bisfilename==false)
  {
    if (access(filename,F_OK) != 0)
    {
      if (mkdir(filename,0755) != 0) return false;
    }
  }

  return true;
}

// 打开文件。
// FOPEN函数调用fopen库函数打开文件，如果文件名中包含的目录不存在，就创建目录。
// FOPEN函数的参数和返回值与fopen函数完全相同。
// 在应用开发中，用FOPEN函数代替fopen库函数。
FILE *FOPEN(const char *filename,const char *mode)
{
  if (MKDIR(filename) == false) return 0;

  return fopen(filename,mode);
}

// 获取文件的大小。
// filename：待获取的文件名，建议采用绝对路径的文件名。
// 返回值：如果文件不存在或没有访问权限，返回-1，成功返回文件的大小，单位是字节。
int FileSize(const char *filename)
{
  struct stat st_filestat;

  if (stat(filename,&st_filestat) < 0) return -1;

  return st_filestat.st_size;
}

// 重置文件的修改时间属性。
// filename：待重置的文件名，建议采用绝对路径的文件名。
// stime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少。
// 返回值：true-成功；false-失败，失败的原因保存在errno中。
bool UTime(const char *filename,const char *mtime)
{
  struct utimbuf stutimbuf;

  stutimbuf.actime=stutimbuf.modtime=strtotime(mtime);

  if (utime(filename,&stutimbuf)!=0) return false;

  return true;
}

// 把字符串表示的时间转换为整数表示的时间。
// stime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少。
// 返回值：整数表示的时间，如果stime的格式不正确，返回-1。
time_t strtotime(const char *stime)
{
  char strtime[21],yyyy[5],mm[3],dd[3],hh[3],mi[3],ss[3];
  memset(strtime,0,sizeof(strtime));
  memset(yyyy,0,sizeof(yyyy));
  memset(mm,0,sizeof(mm));
  memset(dd,0,sizeof(dd));
  memset(hh,0,sizeof(hh));
  memset(mi,0,sizeof(mi));
  memset(ss,0,sizeof(ss));

  PickNumber(stime,strtime,false,false);

  if (strlen(strtime) != 14) return -1;

  strncpy(yyyy,strtime,4);
  strncpy(mm,strtime+4,2);
  strncpy(dd,strtime+6,2);
  strncpy(hh,strtime+8,2);
  strncpy(mi,strtime+10,2);
  strncpy(ss,strtime+12,2);

  struct tm time_str;

  time_str.tm_year = atoi(yyyy) - 1900;
  time_str.tm_mon = atoi(mm) - 1;
  time_str.tm_mday = atoi(dd);
  time_str.tm_hour = atoi(hh);
  time_str.tm_min = atoi(mi);
  time_str.tm_sec = atoi(ss);
  time_str.tm_isdst = 0;

  return mktime(&time_str);
}

// 把字符串表示的时间加上一个偏移的秒数后得到一个新的字符串表示的时间。
// in_stime：输入的字符串格式的时间。
// out_stime：输出的字符串格式的时间。
// timetvl：需要偏移的秒数，正数往后偏移，负数往前偏移。
// fmt：输出字符串时间out_stime的格式，与LocalTime函数的fmt参数相同。
// 注意：in_stime和out_stime参数可以是同一个变量的地址，如果调用失败，out_stime的内容会清空。
// 返回值：true-成功，false-失败，如果返回失败，可以认为是in_stime的格式不正确。
bool AddTime(const char *in_stime,char *out_stime,const int timetvl,const char *fmt)
{
  if ( (in_stime==0) || (out_stime==0) ) return false;    // 判断空指针。

  time_t  timer;
  if ( (timer=strtotime(in_stime))==-1) { strcpy(out_stime,""); return false; }

  timer=timer+timetvl;

  strcpy(out_stime,"");

  timetostr(timer,out_stime,fmt);

  return true;
}

CTimer::CTimer()
{
	memset(&m_start, 0, sizeof(struct timeval));
	memset(&m_end, 0, sizeof(struct timeval));

	Start();
}

void CTimer::Start()
{
	gettimeofday(&m_start, 0);
}

double CTimer::Elapsed()
{
	gettimeofday(&m_end, 0);

	double start, end;
	start = end = 0;

	char strtemp[51];
	SNPRINTF(strtemp, sizeof(strtemp), 30, "%ld.%06ld",
		m_start.tv_sec, m_start.tv_usec);
	start = atof(strtemp);

	SNPRINTF(strtemp, sizeof(strtemp), 30, "%ld.%06ld",
		m_end.tv_sec, m_end.tv_usec);
	end = atof(strtemp);

	Start();
	return end - start;
}

/*************************************************************************************
 * 目录操作相关的类
**************************************************************************************/

CDir::CDir()
{
	m_pos = 0;

	STRCPY(m_DateFMT, sizeof(m_DateFMT), "yyyy-mm-dd hh24:mi:ss");

	m_vFileName.clear();

	initdata();
}

void CDir::initdata()
{
	memset(m_DirName, 0, sizeof(m_DirName));
	memset(m_FileName, 0, sizeof(m_FileName));
	memset(m_FullFileName, 0, sizeof(m_FullFileName));
	m_FileSize = 0;
	memset(m_CreateTime, 0, sizeof(m_CreateTime));
	memset(m_ModifyTime, 0, sizeof(m_ModifyTime));
	memset(m_AccessTime, 0, sizeof(m_AccessTime));
}

void CDir::SetDateFMT(const char *in_DateFMT)
{
	memset(m_DateFMT, 0, sizeof(m_DateFMT));
	STRCPY(m_DateFMT, sizeof(m_DateFMT), in_DateFMT);
}

bool CDir::OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const bool bAndChild,bool bSort)
{
	m_pos = 0;
	m_vFileName.clear();

	// 如果目录不存在，就创建该目录
	if (MKDIR(in_DirName, false) == false) return false;

	bool bRet = _OpenDir(in_DirName, in_MatchStr, in_MaxCount, bAndChild);

	if (bSort == true)
	{
		sort(m_vFileName.begin(), m_vFileName.end());
	}
	return bRet;
}

bool CDir::_OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const bool bAndChild)
{
	DIR *dir;

	if ( (dir=opendir(in_DirName)) == 0 ) return false;

	char strTempFileName[3001];

	struct dirent *st_fileinfo;
	struct stat st_filestat;

	while ( (st_fileinfo=readdir(dir)) != 0)
	{
		// 以"."打头的文件不处理
		if (st_fileinfo->d_name[0] == '.') continue;

		SNPRINTF(strTempFileName, sizeof(strTempFileName), 300, "%s//%s", in_DirName, st_fileinfo->d_name);

		UpdateStr(strTempFileName, "//", "/");

		stat(strTempFileName, &st_filestat);

		// 判断是否是目录，如果是，处理各级子目录。
		if (S_ISDIR(st_filestat.st_mode))
		{
			if (bAndChild == true)
			{
				if (_OpenDir(strTempFileName, in_MatchStr, in_MaxCount, bAndChild) == false)
				{
					closedir(dir); return false;
				}
			}
		}
		else
		{
			// 如果是文件，把能匹配上的文件放入m_vFileName容器中
			if (MatchStr(st_fileinfo->d_name, in_MatchStr) == false) continue;

			m_vFileName.push_back(strTempFileName);

			if ( m_vFileName.size() >= in_MaxCount ) break;
		}
	}

	closedir(dir);

	return true;
}

bool CDir::ReadDir()
{
	initdata();

	int ivsize = m_vFileName.size();

	// 如果已读完，清空容器
	if (m_pos >= ivsize)
	{
		m_pos = 0;
		m_vFileName.clear();
		return false;
	}

	int pos = 0;

	pos = m_vFileName[m_pos].find_last_of("/");

	// 目录名
	STRCPY(m_DirName, sizeof(m_DirName), m_vFileName[m_pos].substr(0, pos).c_str());

	// 文件名
	STRCPY(m_FileName, sizeof(m_FileName), m_vFileName[m_pos].substr(pos+1, m_vFileName[m_pos].size()-pos-1).c_str());

	// 文件全名，包括路径
	SNPRINTF(m_FullFileName, sizeof(m_FullFileName), 300, "%s", m_vFileName[m_pos].c_str());

	struct stat st_filestat;

	stat(m_FullFileName, &st_filestat);

	m_FileSize = st_filestat.st_size;

	struct tm nowtimer;

	if (strcmp(m_DateFMT, "yyyy-mm-dd hh24:mi:ss") == 0)
	{
		nowtimer = *localtime(&st_filestat.st_mtime);

		nowtimer.tm_mon++;
		snprintf(m_ModifyTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

		nowtimer = *localtime(&st_filestat.st_ctime);
		// localtime_r(&st_filestat.st_ctime,&nowtimer); 
		nowtimer.tm_mon++;
		snprintf(m_CreateTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

		nowtimer = *localtime(&st_filestat.st_atime);
		// localtime_r(&st_filestat.st_atime,&nowtimer); 
		nowtimer.tm_mon++;
		snprintf(m_AccessTime,20,"%04u-%02u-%02u %02u:%02u:%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);
	}

	if (strcmp(m_DateFMT,"yyyymmddhh24miss") == 0)
	{
		nowtimer = *localtime(&st_filestat.st_mtime);
		// localtime_r(&st_filestat.st_mtime,&nowtimer); 
		nowtimer.tm_mon++;
		snprintf(m_ModifyTime,20,"%04u%02u%02u%02u%02u%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

		nowtimer = *localtime(&st_filestat.st_ctime);
		// localtime_r(&st_filestat.st_ctime,&nowtimer); 
		nowtimer.tm_mon++;
		snprintf(m_CreateTime,20,"%04u%02u%02u%02u%02u%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

		nowtimer = *localtime(&st_filestat.st_atime);
		// localtime_r(&st_filestat.st_atime,&nowtimer); 
		nowtimer.tm_mon++;
		snprintf(m_AccessTime,20,"%04u%02u%02u%02u%02u%02u",\
				nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
				nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);
	}

	m_pos++;

	return true;
}

CDir::~CDir()  // 析构函数。
{
	m_vFileName.clear();
}

bool REMOVE(const char *filename, const int times)
{
	// 如果文件不存在。直接返回失败
	if (access(filename, R_OK) != 0) return false;

	for (int ii=0; ii < times; ii++)
	{
		if (remove(filename) == 0) return true;

		usleep(100000);
	}
	return false;
}

bool RENAME(const char *srcfilename, const char *dstfilename, const int times)
{
	// 如果文件不存在，直接返回失败
	if (access(srcfilename, R_OK) != 0) return false;

	if (MKDIR(dstfilename) == false) return false;

	for (int i = 0; i < times; i++)
	{
		if (rename(srcfilename, dstfilename) == 0) return true;

		usleep(100000);
	}

	return false;
}

bool COPY(const char *srcfilename,const char *dstfilename)
{
	// 如果文件不存在，直接返回失败
	ifstream srcfile(srcfilename);
    if (!srcfile.is_open()) return false;

	ofstream dstfile(dstfilename);
	char temp[1000];
	while(!srcfile.eof())
	{
		memset(temp, 0, sizeof(temp));
		srcfile.getline(temp, 1000);
		dstfile << temp;
	}
	srcfile.close();
	dstfile.close();

	return true;
}

// 获取文件的时间。
// filename：待获取的文件名，建议采用绝对路径的文件名。
// mtime：用于存放文件的时间，即stat结构体的st_mtime。
// fmt：设置时间的输出格式，与LocalTime函数相同，但缺省是"yyyymmddhh24miss"。
// 返回值：如果文件不存在或没有访问权限，返回false，成功返回true。
bool FileMTime(const char *filename,char *mtime,const char *fmt)
{
  // 判断文件是否存在。
  struct stat st_filestat;

  if (stat(filename,&st_filestat) < 0) return false;

  char strfmt[25];
  memset(strfmt,0,sizeof(strfmt));
  if (fmt==0) strcpy(strfmt,"yyyymmddhh24miss");
  else strcpy(strfmt,fmt);

  timetostr(st_filestat.st_mtime,mtime,strfmt);

  return true;
}

CFile::CFile()   // 类的构造函数
{
	m_fp=0;
	m_bEnBuffer=true;
	memset(m_filename,0,sizeof(m_filename));
	memset(m_filenametmp,0,sizeof(m_filenametmp));
}

// 关闭文件指针
void CFile::Close() 
{
	if (m_fp==0) return;    // 判断空指针。

	fclose(m_fp);  // 关闭文件指针

	m_fp=0;
	memset(m_filename,0,sizeof(m_filename));

	// 如果存在临时文件，就删除它。
	if (strlen(m_filenametmp)!=0) remove(m_filenametmp);

	memset(m_filenametmp,0,sizeof(m_filenametmp));
}

// 判断文件是否已打开
bool CFile::IsOpened()
{
	if (m_fp==0) return false;    // 判断空指针。

	return true;
}

// 关闭文件指针，并删除文件
bool CFile::CloseAndRemove()
{
	if (m_fp==0) return true;    // 判断空指针。

	fclose(m_fp);  // 关闭文件指针

	m_fp=0;

	if (remove(m_filename) != 0) { memset(m_filename,0,sizeof(m_filename)); return false; }

	memset(m_filename,0,sizeof(m_filename));
	memset(m_filenametmp,0,sizeof(m_filenametmp));

	return true;
}

CFile::~CFile()   // 类的析构函数
{
	Close();
}

// 打开文件，参数与FOPEN相同，打开成功true，失败返回false
bool CFile::Open(const char *filename,const char *openmode,bool bEnBuffer)
{
	Close();

	if ( (m_fp=FOPEN(filename,openmode)) == 0 ) return false;

	memset(m_filename,0,sizeof(m_filename));

	STRNCPY(m_filename,sizeof(m_filename),filename,300);

	m_bEnBuffer=bEnBuffer;

	return true;
}

// 专为改名而打开文件，参数与fopen相同，打开成功true，失败返回false
bool CFile::OpenForRename(const char *filename,const char *openmode,bool bEnBuffer)
{
	Close();

	memset(m_filename,0,sizeof(m_filename));
	STRNCPY(m_filename,sizeof(m_filename),filename,300);
	
	memset(m_filenametmp,0,sizeof(m_filenametmp));
	SNPRINTF(m_filenametmp,sizeof(m_filenametmp),300,"%s.tmp",m_filename);

	if ( (m_fp=FOPEN(m_filenametmp,openmode)) == 0 ) return false;

	m_bEnBuffer=bEnBuffer;

	return true;
}

// 关闭文件并改名
bool CFile::CloseAndRename()
{
	if (m_fp==0) return false;    // 判断空指针。

	fclose(m_fp);  // 关闭文件指针

	m_fp=0;

	if (rename(m_filenametmp,m_filename) != 0)
	{
		remove(m_filenametmp);
		memset(m_filename,0,sizeof(m_filename));
		memset(m_filenametmp,0,sizeof(m_filenametmp));
		return false;
	}

	memset(m_filename,0,sizeof(m_filename));
	memset(m_filenametmp,0,sizeof(m_filenametmp));

	return true;
}

// 调用fprintf向文件写入数据
void CFile::Fprintf(const char *fmt,...)
{
	if ( m_fp == 0 ) return;

	va_list arg;
	va_start( arg, fmt );
	vfprintf( m_fp, fmt, arg );
	va_end( arg );

	if ( m_bEnBuffer == false ) fflush(m_fp);
}

// 调用fgets从文件中读取一行，bDelCRT=true删除换行符，false不删除，缺省为false
bool CFile::Fgets(char *buffer,const int readsize,bool bdelcrt)
{
	if ( m_fp == 0 ) return false;

	memset(buffer,0,readsize+1);  // 调用者必须保证buffer的空间足够，否则这里会内存溢出。

	if (fgets(buffer,readsize,m_fp) == 0) return false;

	if (bdelcrt==true)
	{
		DeleteRChar(buffer,'\n'); DeleteRChar(buffer,'\r');  // 如果文件是windows格式，还要删除'\r'。
	}

	return true;
}

// 从文件文件中读取一行
// strEndStr是一行数据的结束标志，如果为空，则以换行符"\n"为结束标志。
bool CFile::FFGETS(char *buffer,const int readsize,const char *endbz)
{
	if ( m_fp == 0 ) return false;

	return FGETS(m_fp,buffer,readsize,endbz);
}

// 调用fread从文件中读取数据。
size_t CFile::Fread(void *ptr, size_t size)
{
	if ( m_fp == 0 ) return -1;

	return fread(ptr,1,size,m_fp);
}

// 调用fwrite向文件中写数据
size_t CFile::Fwrite(const void *ptr, size_t size )
{
	if ( m_fp == 0 ) return -1;

	size_t tt=fwrite(ptr,1,size,m_fp);

	if ( m_bEnBuffer == false ) fflush(m_fp);

	return tt;
}

// 从文本文件中读取一行。
// fp：已打开的文件指针。
// buffer：用于存放读取的内容。
// readsize：本次打算读取的字节数，如果已经读取到了结束标志，函数返回。
// endbz：行内容结束的标志，缺省为空，表示行内容以"\n"为结束标志。
// 返回值：true-成功；false-失败，一般情况下，失败可以认为是文件已结束。
bool FGETS(const FILE *fp,char *buffer,const int readsize,const char *endbz)
{
	if ( fp == 0 ) return false;

	memset(buffer,0,readsize+1);   // 调用者必须保证buffer的空间足够，否则这里会内存溢出。

	char strline[readsize+1];

	while (true)
	{
		memset(strline,0,sizeof(strline));

		if (fgets(strline,readsize,(FILE *)fp) == 0) break;

		// 防止buffer溢出
		if ( (strlen(buffer)+strlen(strline)) >= (unsigned int)readsize ) break;

		strcat(buffer,strline);

		if (endbz == 0) return true;
		else if (strstr(strline,endbz)!= 0) return true;
		// 以上代码可以不用strstr，可以优化为只比较最后的内容。
	}

	return false;
}


CCmdStr::CCmdStr()
{
	m_vCmdStr.clear();
}

CCmdStr::CCmdStr(const string &buffer,const char *sepstr,const bool bdelspace)
{
	m_vCmdStr.clear();

	SplitToCmd(buffer,sepstr,bdelspace);
}

// 把字符串拆分到m_vCmdStr容器中。
// buffer：待拆分的字符串。
// sepstr：buffer字符串中字段内容的分隔符，注意，分隔符是字符串，如","、" "、"|"、"~!~"。
// bdelspace：是否删除拆分后的字段内容前后的空格，true-删除；false-不删除，缺省不删除。
void CCmdStr::SplitToCmd(const string &buffer,const char *sepstr,const bool bdelspace)
{
	// 清除所有的旧数据
	m_vCmdStr.clear();

	int iPOS=0;
	string srcstr,substr;

	srcstr=buffer;

	while ( (iPOS=srcstr.find(sepstr)) >= 0)
	{
		substr=srcstr.substr(0,iPOS);

		if (bdelspace == true)
		{
		char str[substr.length()+1];
		STRCPY(str,sizeof(str),substr.c_str());

		DeleteLRChar(str,' ');

		substr=str;
		}

		m_vCmdStr.push_back(substr);

		iPOS=iPOS+strlen(sepstr);

		srcstr=srcstr.substr(iPOS,srcstr.size()-iPOS);

	}

	substr=srcstr;

	if (bdelspace == true)
	{
		char str[substr.length()+1];
		STRCPY(str,sizeof(str),substr.c_str());

		DeleteLRChar(str,' ');

		substr=str;
	}

	m_vCmdStr.push_back(substr);

	return;
}

int CCmdStr::CmdCount()
{
	return m_vCmdStr.size();
}

bool CCmdStr::GetValue(const int inum,char *value,const int ilen)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	if (ilen>0) memset(value,0,ilen+1);   // 调用者必须保证value的空间足够，否则这里会内存溢出。

	if ( (m_vCmdStr[inum].length()<=(unsigned int)ilen) || (ilen==0) )
	{
		strcpy(value,m_vCmdStr[inum].c_str());
	}
	else
	{
		strncpy(value,m_vCmdStr[inum].c_str(),ilen); value[ilen]=0;
	}

	return true;
}

bool CCmdStr::GetValue(const int inum,int *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	(*value) = atoi(m_vCmdStr[inum].c_str());

	return true;
}

bool CCmdStr::GetValue(const int inum,unsigned int *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	(*value) = atoi(m_vCmdStr[inum].c_str());

	return true;
}


bool CCmdStr::GetValue(const int inum,long *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	(*value) = atol(m_vCmdStr[inum].c_str());

	return true;
}

bool CCmdStr::GetValue(const int inum,unsigned long *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	(*value) = atol(m_vCmdStr[inum].c_str());

	return true;
}

bool CCmdStr::GetValue(const int inum,double *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	(*value) = (double)atof(m_vCmdStr[inum].c_str());

	return true;
}

bool CCmdStr::GetValue(const int inum,bool *value)
{
	if ( (inum>=(int)m_vCmdStr.size()) || (value==0) ) return false;

	(*value) = 0;

	if (inum >= (int)m_vCmdStr.size()) return false;

	char strTemp[11];
	memset(strTemp,0,sizeof(strTemp));
	strncpy(strTemp,m_vCmdStr[inum].c_str(),10);

	ToUpper(strTemp);  // 转换为大写来判断。
	if (strcmp(strTemp,"TRUE")==0) (*value)=true; 

	return true;
}

CCmdStr::~CCmdStr()
{
	m_vCmdStr.clear();
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,char *value,const int ilen)
{
	if (value==0) return false;    // 判断空指针。

	if (ilen>0) memset(value,0,ilen+1);   // 调用者必须保证value的空间足够，否则这里会内存溢出。

	char *start=0,*end=0;
	char m_SFieldName[51],m_EFieldName[51];

	int m_NameLen = strlen(fieldname);

	SNPRINTF(m_SFieldName,sizeof(m_SFieldName),50,"<%s>",fieldname);
	SNPRINTF(m_EFieldName,sizeof(m_EFieldName),50,"</%s>",fieldname);

	start=0; end=0;

	start = (char *)strstr(xmlbuffer,m_SFieldName);

	if (start != 0)
	{
		end   = (char *)strstr(start,m_EFieldName);
	}

	if ((start==0) || (end == 0))
	{
		return false;
	}

	int   m_ValueLen = end - start - m_NameLen - 2 ;

	if ( ((m_ValueLen) <= ilen) || (ilen == 0) )
	{
		strncpy(value,start+m_NameLen+2,m_ValueLen); value[m_ValueLen]=0;
	}
	else
	{
		strncpy(value,start+m_NameLen+2,ilen); value[ilen]=0;
	}

	DeleteLRChar(value,' ');

	return true;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,bool *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = false;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,10) == true)
	{
		ToUpper(strTemp);  // 转换为大写来判断。
		if (strcmp(strTemp,"TRUE")==0) { (*value)=true; return true; }
	}

	return false;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,int *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = 0;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,50) == true)
	{
		(*value) = atoi(strTemp); return true;
	}

	return false;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,unsigned int *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = 0;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,50) == true)
	{
		(*value) = (unsigned int)atoi(strTemp); return true;
	}

	return false;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,long *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = 0;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,50) == true)
	{
		(*value) = atol(strTemp); return true;
	}

	return false;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,unsigned long *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = 0;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,50) == true)
	{
		(*value) = (unsigned long)atol(strTemp); return true;
	}

	return false;
}

bool GetXMLBuffer(const char *xmlbuffer,const char *fieldname,double *value)
{
	if (value==0) return false;    // 判断空指针。

	(*value) = 0;

	char strTemp[51];

	memset(strTemp,0,sizeof(strTemp));

	if (GetXMLBuffer(xmlbuffer,fieldname,strTemp,50) == true)
	{
		(*value) = atof(strTemp); return true;
	}

	return false;
}

void timetostr(const time_t ltime,char *stime,const char *fmt)
{
	if (stime==0) return;    // 判断空指针。

	strcpy(stime,"");

	struct tm sttm = *localtime ( &ltime );
	// struct tm sttm; localtime_r(&ltime,&sttm); 

	sttm.tm_year=sttm.tm_year+1900;
	sttm.tm_mon++;
	
	if (fmt==0)
	{
		snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
						sttm.tm_min,sttm.tm_sec);
		return;
	}

	if (strcmp(fmt,"yyyy-mm-dd hh24:mi:ss") == 0)
	{
		snprintf(stime,20,"%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
						sttm.tm_min,sttm.tm_sec);
		return;
	}

	if (strcmp(fmt,"yyyy-mm-dd hh24:mi") == 0)
	{
		snprintf(stime,17,"%04u-%02u-%02u %02u:%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
						sttm.tm_min);
		return;
	}

	if (strcmp(fmt,"yyyy-mm-dd hh24") == 0)
	{
		snprintf(stime,14,"%04u-%02u-%02u %02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
		return;
	}

	if (strcmp(fmt,"yyyy-mm-dd") == 0)
	{
		snprintf(stime,11,"%04u-%02u-%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
		return;
	}

	if (strcmp(fmt,"yyyy-mm") == 0)
	{
		snprintf(stime,8,"%04u-%02u",sttm.tm_year,sttm.tm_mon); 
		return;
	}

	if (strcmp(fmt,"yyyymmddhh24miss") == 0)
	{
		snprintf(stime,15,"%04u%02u%02u%02u%02u%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
						sttm.tm_min,sttm.tm_sec);
		return;
	}

	if (strcmp(fmt,"yyyymmddhh24mi") == 0)
	{
		snprintf(stime,13,"%04u%02u%02u%02u%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour,
						sttm.tm_min);
		return;
	}

	if (strcmp(fmt,"yyyymmddhh24") == 0)
	{
		snprintf(stime,11,"%04u%02u%02u%02u",sttm.tm_year,
						sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
		return;
	}

	if (strcmp(fmt,"yyyymmdd") == 0)
	{
		snprintf(stime,9,"%04u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
		return;
	}

	if (strcmp(fmt,"hh24miss") == 0)
	{
		snprintf(stime,7,"%02u%02u%02u",sttm.tm_hour,sttm.tm_min,sttm.tm_sec); 
		return;
	}

	if (strcmp(fmt,"hh24mi") == 0)
	{
		snprintf(stime,5,"%02u%02u",sttm.tm_hour,sttm.tm_min); 
		return;
	}

	if (strcmp(fmt,"hh24") == 0)
	{
		snprintf(stime,3,"%02u",sttm.tm_hour); 
		return;
	}

	if (strcmp(fmt,"mi") == 0)
	{
		snprintf(stime,3,"%02u",sttm.tm_min); 
		return;
	}
}


/*
  取操作系统的时间，并把整数表示的时间转换为字符串表示的格式。
  stime：用于存放获取到的时间字符串。
  timetvl：时间的偏移量，单位：秒，0是缺省值，表示当前时间，30表示当前时间30秒之后的时间点，-30表示当前时间30秒之前的时间点。
  fmt：输出时间的格式，缺省是"yyyy-mm-dd hh24:mi:ss"，目前支持以下格式：
  "yyyy-mm-dd hh24:mi:ss"，此格式是缺省格式。
  "yyyymmddhh24miss"
  "yyyy-mm-dd"
  "yyyymmdd"
  "hh24:mi:ss"
  "hh24miss"
  "hh24:mi"
  "hh24mi"
  "hh24"
  "mi"
  注意：
    1）小时的表示方法是hh24，不是hh，这么做的目的是为了保持与数据库的时间表示方法一致；
    2）以上列出了常用的时间格式，如果不能满足你应用开发的需求，请修改源代码增加更多的格式支持；
    3）调用函数的时候，如果fmt与上述格式都匹配，stime的内容将为空。
*/
void LocalTime(char *stime, const char *fmt, const int timetvl)
{
	if (stime==0) return;    // 判断空指针。

	time_t  timer;

	time( &timer ); timer=timer+timetvl;
	
	timetostr(timer,stime,fmt);
}

/*****************************************************************
 * 日志文件操作类成员函数
*******************************************************************/

CLogFile::CLogFile(const long MaxLogSize)
{
	m_tracefp = 0;
	memset(m_filename,0,sizeof(m_filename));
	memset(m_openmode,0,sizeof(m_openmode));
	m_bBackup=true;
	m_bEnBuffer=false;
	m_MaxLogSize=MaxLogSize;
	if (m_MaxLogSize<10) m_MaxLogSize=10;

	// pthread_pin_init(&spin,0);  // 初学暂时不要关心这行代码。
}

CLogFile::~CLogFile()
{
	Close();

	// pthread_spin_destroy(&spin);  // 初学暂时不要关心这行代码。
}

void CLogFile::Close()
{
	if (m_tracefp != 0) { fclose(m_tracefp); m_tracefp=0; }

	memset(m_filename,0,sizeof(m_filename));
	memset(m_openmode,0,sizeof(m_openmode));
	m_bBackup=true;
	m_bEnBuffer=false;
}

// 打开日志文件。
// filename：日志文件名，建议采用绝对路径，如果文件名中的目录不存在，就先创建目录。
// openmode：日志文件的打开方式，与fopen库函数打开文件的方式相同，缺省值是"a+"。
// bBackup：是否自动切换，true-切换，false-不切换，在多进程的服务程序中，如果多个进行共用一个日志文件，bBackup必须为false。
// bEnBuffer：是否启用文件缓冲机制，true-启用，false-不启用，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件，缺省是不启用。
bool CLogFile::Open(const char *filename,const char *openmode,bool bBackup,bool bEnBuffer)
{
	// 如果文件指针是打开的状态，先关闭它。
	Close();

	STRCPY(m_filename,sizeof(m_filename),filename);
	m_bEnBuffer=bEnBuffer;
	m_bBackup=bBackup;
	if (openmode==0) STRCPY(m_openmode,sizeof(m_openmode),"a+");
	else STRCPY(m_openmode,sizeof(m_openmode),openmode);

	if ((m_tracefp=FOPEN(m_filename,m_openmode)) == 0) return false;

	return true;
}

// 如果日志文件大于100M，就把当前的日志文件备份成历史日志文件，切换成功后清空当前日志文件的内容。
// 备份后的文件会在日志文件名后加上日期时间。
// 注意，在多进程的程序中，日志文件不可切换，多线的程序中，日志文件可以切换。
bool CLogFile::BackupLogFile()
{
	if (m_tracefp == 0) return false;

	// 不备份
	if (m_bBackup == false) return true;

	//fseek(m_tracefp,0,2);

	// 文件指针在文件中的位置，若大于定义的大小，则更换文件
	if (ftell(m_tracefp) > m_MaxLogSize*1024*1024)
	{
		// 关闭文件，然后将文件指针置为0
		fclose(m_tracefp); m_tracefp=0;

		char strLocalTime[21];
		memset(strLocalTime, 0, sizeof(strLocalTime));
		LocalTime(strLocalTime, "yyyymmddhh24miss");

		char bak_filename[301];
		SNPRINTF(bak_filename,sizeof(bak_filename),300,"%s.%s",m_filename,strLocalTime);
		// 文件重命名，将老文件名改为带时间的日志文件名
		rename(m_filename,bak_filename);

		if ((m_tracefp=FOPEN(m_filename,m_openmode)) == 0) return false;
	}

	return true;
}

// 把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同。
// Write方法会写入当前的时间，WriteEx方法不写时间。
bool CLogFile::Write(const char *fmt,...)
{
	if (m_tracefp == 0) return false;

	// pthread_spin_lock(&spin);  // 初学暂时不要关心这行代码。

	if (BackupLogFile() == false) return false;

	char strtime[20]; LocalTime(strtime);
	// 可变参数列表
	va_list ap;
	va_start(ap,fmt);
	fprintf(m_tracefp,"%s ",strtime);
	vfprintf(m_tracefp,fmt,ap);
	va_end(ap);

	if (m_bEnBuffer==false) fflush(m_tracefp);

	// pthread_spin_unlock(&spin);  // 初学暂时不要关心这行代码。

	return true;
}

// 把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同。
// Write方法会写入当前的时间，WriteEx方法不写时间。
bool CLogFile::WriteEx(const char *fmt,...)
{
	if (m_tracefp == 0) return false;

	// pthread_spin_lock(&spin);  // 初学暂时不要关心这行代码。

	va_list ap;
	va_start(ap,fmt);
	vfprintf(m_tracefp,fmt,ap);
	va_end(ap);

	if (m_bEnBuffer==false) fflush(m_tracefp);

	// pthread_spin_unlock(&spin);  // 初学暂时不要关心这行代码。

	return true;
}

CSEM::CSEM()
{
	m_semid=-1;
	m_sem_flg=SEM_UNDO;
}

// 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
bool CSEM::init(key_t key,unsigned short value,short sem_flg)
{
	if (m_semid!=-1) return false;

	m_sem_flg=sem_flg;

	// 信号量的初始化不能直接用semget(key,1,0666|IPC_CREAT)，因为信号量创建后，初始值是0。

	// 信号量的初始化分三个步骤：
	// 1）获取信号量，如果成功，函数返回。
	// 2）如果失败，则创建信号量。
	// 3) 设置信号量的初始值。

	// 1）获取信号量，如果成功，函数返回
	if ( (m_semid=semget(key,1,0666)) == -1)
	{
		// 如果信号量不存在，创建它。
		if (errno==2)		// 当前键值的信号量集不存在，且没有指定参数 IPC_CREAT
		{
			// 用IPC_EXCL标志确保只有一个进程创建并初始化信号量，其它进程只能获取。
			if ( (m_semid=semget(key,1,0666|IPC_CREAT|IPC_EXCL)) == -1)
			{
				// 若当前键值的信号量集不存在
				if (errno!=EEXIST) 
				{
				perror("init 1 semget()"); return false;
				}
				if ( (m_semid=semget(key,1,0666)) == -1)
				{ perror("init 2 semget()"); return false; }
			
				return true;
			}

			// 信号量创建成功后，还需要把它初始化成value。
			union semun sem_union;
			sem_union.val = value;   // 设置信号量的初始值。
			if (semctl(m_semid,0,SETVAL,sem_union) <  0) { perror("init semctl()"); return false; }
		}
		else
		{ perror("init 3 semget()"); return false; }
	}

	return true;
}

bool CSEM::P(short sem_op)
{
	if (m_semid==-1) return false;

	struct sembuf sem_b;
	sem_b.sem_num = 0;      // 信号量编号，0代表第一个信号量。
	sem_b.sem_op = sem_op;  // P操作的sem_op必须小于0。
	sem_b.sem_flg = m_sem_flg;   
	if (semop(m_semid,&sem_b,1) == -1) { perror("p semop()"); return false; }

	return true;
}

bool CSEM::V(short sem_op)
{
	if (m_semid==-1) return false;

	struct sembuf sem_b;
	sem_b.sem_num = 0;      // 信号量编号，0代表第一个信号量。
	sem_b.sem_op = sem_op;  // V操作的sem_op必须大于0。
	sem_b.sem_flg = m_sem_flg;
	if (semop(m_semid,&sem_b,1) == -1) { perror("V semop()"); return false; }

	return true;
}

// 获取信号量的值，成功返回信号量的值，失败返回-1。
int CSEM::value()
{
	return semctl(m_semid,0,GETVAL);
}

bool CSEM::destroy()
{
	if (m_semid==-1) return false;

	if (semctl(m_semid,0,IPC_RMID) == -1) { perror("destroy semctl()"); return false; }

	return true;
}

CSEM::~CSEM()
{
}

void CloseIOAndSignal(bool bCloseIO)
{
	for (int i=0; i< 64; i++)
	{
		if (bCloseIO == true)
			close(i);
		signal(i, SIG_IGN);
	}
}


CPActive::CPActive()
{
	m_shmid = -1;
	m_pos = -1;
	m_shm = 0;
}

bool CPActive::AddPInfo(const int timeout, const char *pname)
{
	if ( (m_shmid=shmget(SHMKEYP_, MAXNUMP_*sizeof(struct st_pinfo), 0640|IPC_CREAT)) == -1)
	{
		printf("shmget(%x) failed\n", SHMKEYP_); return false;
	}

	if ( (m_sem.init(SEMKEYP_) == false))
	{
		printf("m_sem.init(%x) failed\n", SEMKEYP_);
		return false;
	}

	// 将共享内存连接到当前进程的地址空间
	m_shm = (struct st_pinfo *)shmat(m_shmid, 0, 0);
	// cout << m_shm << endl;
	// 创建当前进程心跳信息结构体变量，把本进程的信息填进去
	struct st_pinfo stpinfo;
	memset(&stpinfo, 0, sizeof(struct st_pinfo));
	stpinfo.pid = getpid();
	STRNCPY(stpinfo.pname, sizeof(stpinfo.pname), pname, 50);
	stpinfo.timeout = timeout;
	stpinfo.atime = time(0);
	
	// TODO 该行代码会导致m_pos的值在该函数外一直为-1，原因未知
	// int m_pos = -1;

	/**
	 * 进程id是循环使用的，若存在进程异常退出，没有清理心跳信息，进程信息会残留在共享内存中，导致误杀
	*/
	// 若共享内存中存在当前进程编号，当前进程则重用该位置
	for (int ii = 0; ii < MAXNUMP_; ii++)
	{
		if (m_shm[ii].pid == stpinfo.pid) { m_pos=ii; break; }
	}

	m_sem.P();
	// 在共享内存中查找一个空位置，把当前进程的心跳信息存入共享内存中
	if (m_pos == -1)
	{
		for (int ii=0; ii < MAXNUMP_; ii++)
		{
			if ((m_shm+ii)->pid == 0)
			{
				// cout << m_pos << endl;
				m_pos = ii; break;
			}
		}
	}
	// cout << m_pos << endl;
	if (m_pos == -1)
	{
		m_sem.V();
		printf("共享内存空间已用完。\n");
		return false;
	}

	// 把当前进程的心跳信息存入共享内存的进程组中
	memcpy(m_shm+m_pos, &stpinfo, sizeof(struct st_pinfo));

	m_sem.V();
	// cout << "m_sem.V after:" << m_pos << endl;
	return true;
}

bool CPActive::UptATime()
{
	// cout << "UptATime:" << m_pos << endl;
	// 更新共享内存中本进程的心跳时间
	if (m_pos == -1)
	{
	 	return false;
	}	
	m_shm[m_pos].atime = time(0);
	return true;
}

CPActive::~CPActive()
{
	// 把当前进程从共享内存中移去
	m_shm[m_pos].pid = 0;
	memset(m_shm+m_pos, 0, sizeof(struct st_pinfo));

	// 把共享内存从当前进程中分离
	shmdt(m_shm);
}

/****************************************************************************************************
 * @brief SOCKET相关函数封装
 * 
 *****************************************************************************************************/

bool Readn(const int sockfd,char *buffer,const size_t n)
{
  int nLeft=n;  // 剩余需要读取的字节数。
  int idx=0;    // 已成功读取的字节数。
  int nread;    // 每次调用recv()函数读到的字节数。

  while(nLeft > 0)
  {
    if ( (nread=recv(sockfd,buffer+idx,nLeft,0)) <= 0) return false;

    idx=idx+nread;
    nLeft=nLeft-nread;
  }

  return true;
}

bool Writen(const int sockfd,const char *buffer,const size_t n)
{
  int nLeft=n;  // 剩余需要写入的字节数。
  int idx=0;    // 已成功写入的字节数。
  int nwritten; // 每次调用send()函数写入的字节数。
  
  while(nLeft > 0 )
  {    
    if ( (nwritten=send(sockfd,buffer+idx,nLeft,0)) <= 0) return false;      
    
    nLeft=nLeft-nwritten;
    idx=idx+nwritten;
  }

  return true;
}

bool TcpRead(const int sockfd, char *buffer, int *ibuflen, const int itimeout)
{
	if (sockfd==-1) return false;

	// 如果itimeout>0，表示需要等待itimeout秒，如果itimeout秒后还没有数据到达，返回false。
	if (itimeout>0)
	{
		struct pollfd fds;
		fds.fd=sockfd;
		fds.events=POLLIN;
		if ( poll(&fds,1,itimeout*1000) <= 0 ) return false;
	}

	// 如果itimeout==-1，表示不等待，立即判断socket的缓冲区中是否有数据，如果没有，返回false。
	if (itimeout==-1)
	{
		struct pollfd fds;
		fds.fd=sockfd;
		fds.events=POLLIN;
		if ( poll(&fds,1,0) <= 0 ) return false;
	}

	(*ibuflen) = 0;  // 报文长度变量初始化为0。

	// 先读取报文长度，4个字节。
	if (Readn(sockfd,(char*)ibuflen,4) == false) return false;

	(*ibuflen)=ntohl(*ibuflen);  // 把报文长度由网络字节序转换为主机字节序。

	// 再读取报文内容。
	if (Readn(sockfd,buffer,(*ibuflen)) == false) return false;

	return true;
}

bool TcpWrite(const int sockfd, const char *buffer, const int ibuflen)
{
	if (sockfd == -1) return false;

	int ilen = 0; // 报文长度。

	// 如果ibuflen为0，则认为需要发送的是字符串，报文长度为字符串的长度
	if (ibuflen == 0) ilen = strlen(buffer);
	else ilen = ibuflen;

	int ilenn = htonl(ilen);		//把报文长度转换为网络字节序

	char TBuffer[ilen+4];				// 发送数据缓冲区
	memset(TBuffer, 0, sizeof(TBuffer)); // 清空发送缓冲区
	memcpy(TBuffer, &ilenn, 4);			 // 把报文长度拷贝到缓冲区
	memcpy(TBuffer+4, buffer, ilen);	 // 把报文内容拷贝到缓冲区

	// 发送缓冲区中的数据
	if (Writen(sockfd, TBuffer, ilen+4) == false) return false;

	return true;
}

TcpClient::TcpClient()
{
	m_connfd = -1;
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;
	m_btimeout = false;
}

bool TcpClient::ConnectToServer(const char *ip, const int port)
{
	// 如果已连接到服务端，则断开
	if (m_connfd != -1) { close(m_connfd); m_connfd = -1; }

	// 忽略SIGPIPE信号，防止程序异常退出
	// 避免SIGPIPE信号关闭程序
	signal(SIGPIPE, SIG_IGN);

	STRCPY(m_ip, sizeof(m_ip), ip);
	m_port = port;

	struct hostent* h;
	struct sockaddr_in servaddr;

	if ( (m_connfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0)
		return false;
	
	// TODO
	if ( !(h = gethostbyname(m_ip)) )
	{
		close(m_connfd);
		m_connfd = -1;
		return false;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(m_port);	// 指定服务端的通讯端口
	memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

	if (connect(m_connfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		close(m_connfd); m_connfd = -1; return false;
	}

	return true;
}

bool TcpClient::Read(char *buffer, const int itimeout)
{
	if (m_connfd == -1) return false;

	// 如果itimeout>0,表示需要等待itimeout秒
	if (itimeout > 0)
	{
		// TODO
		struct pollfd fds;
		fds.fd = m_connfd;
		fds.events = POLLIN;
		int iret;
		m_btimeout = false;
		if ( (iret = poll(&fds, 1, itimeout*1000)) <= 0)
		{
			if (iret == 0) m_btimeout = true;
			return false;
		}
	}

	m_buflen = 0;
	return (TcpRead(m_connfd, buffer, &m_buflen));
}

bool TcpClient::Write(const char *buffer, const int ibuflen)
{
	if (m_connfd == -1) return false;

	int ilen = ibuflen;

	if (ibuflen == 0) ilen = strlen(buffer);

	return (TcpWrite(m_connfd, buffer, ilen));
}

void TcpClient::Close()
{
	if (m_connfd > 0) close(m_connfd);

	m_connfd = -1;
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;
	m_btimeout=false;
}

TcpClient::~TcpClient()
{
	Close();
}

TcpServer::TcpServer()
{
	m_listenfd = -1;
	m_connfd = -1;
	m_socklen = 0;
	m_btimeout = false;
}

bool TcpServer::InitServer(const unsigned int port, const int backlog)
{
	// 如果服务端的socket>0, 关掉它
	if (m_listenfd > 0) { close(m_listenfd); m_listenfd = -1; }

	if ( (m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) return false;

	// 忽略SIGPIPE信号
	signal(SIGPIPE, SIG_IGN);

	// 打开SO_REUSEADDR选项，当服务端连接处于TIME_WAIT状态时可以再次启动服务器，
	// 否则bind()可能会不成功，报：Address already in use。
	int opt = 1; unsigned int len = sizeof(opt);
	setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

	memset(&m_servaddr, 0, sizeof(m_servaddr));
	m_servaddr.sin_family = AF_INET;
	m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_servaddr.sin_port = htons(port);

	if ( bind(m_listenfd, (struct sockaddr *)&m_servaddr, sizeof(m_servaddr)) != 0)
	{
		CloseListen(); return false;
	}

	if (listen(m_listenfd, backlog) != 0)
	{
		CloseListen(); return false;
	}

	return true;
}

bool TcpServer::Accept()
{
	if (m_listenfd==-1) return false;

	m_socklen = sizeof(struct sockaddr_in);

	if ((m_connfd=accept(m_listenfd,(struct sockaddr *)&m_clientaddr,(socklen_t*)&m_socklen)) < 0)
		return false;

	return true;
}

char *TcpServer::GetIP()
{
	return(inet_ntoa(m_clientaddr.sin_addr));
}

bool TcpServer::Read(char *buffer,const int itimeout)
{
	if (m_connfd==-1) return false;

	// 如果itimeout>0，表示需要等待itimeout秒，如果itimeout秒后还没有数据到达，返回false。
	if (itimeout>0)
	{
		struct pollfd fds;
		fds.fd=m_connfd;
		fds.events=POLLIN;
		m_btimeout=false;
		int iret;
		if ( (iret=poll(&fds,1,itimeout*1000)) <= 0 )
		{
		if (iret==0) m_btimeout = true;
		return false;
		}
	}

	m_buflen = 0;
	return(TcpRead(m_connfd,buffer,&m_buflen));
}

bool TcpServer::Write(const char *buffer,const int ibuflen)
{
	if (m_connfd==-1) return false;

	int ilen = ibuflen;
	if (ilen==0) ilen=strlen(buffer);

	return(TcpWrite(m_connfd,buffer,ilen));
}

void TcpServer::CloseListen()
{
	if (m_listenfd > 0)
	{
		close(m_listenfd); m_listenfd=-1;
	}
}

void TcpServer::CloseClientSocket()
{
	if (m_connfd > 0)
	{
		close(m_connfd); m_connfd=-1; 
	}
}

TcpServer::~TcpServer()
{
	CloseListen(); CloseClientSocket();
}