/**
 * @file _mysql.cpp
 * @author your name (you@domain.com)
 * @brief mysql数据库的定义文件
 * @version 0.1
 * @date 2022-09-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "_mysql.h"

void Connection::setdbopt(const char *connstr)
{
	memset(&m_env, 0, sizeof(LOGINENV));

	char *bpos, *epos;
	bpos = epos = 0;

	bpos = (char*)connstr;
	epos = strstr(bpos, ",");
	if (epos > 0)
	{
		strncpy(m_env.ip, bpos, epos - bpos);
	}
	else return;

	bpos = epos + 1;
	epos = 0;
	epos = strstr(bpos, ",");
	if (epos > 0)
	{
		strncpy(m_env.user, bpos, epos - bpos);
	}
	else return;

	bpos = epos + 1;
	epos = 0;
	epos = strstr(bpos, ",");
	if (epos > 0)
	{
		strncpy(m_env.pwd, bpos, epos - bpos);
	}
	else return;

	bpos = epos + 1;
	epos = 0;
	epos = strstr(bpos, ",");
	if (epos > 0)
	{
		strncpy(m_env.dbname, bpos, epos - bpos);
	}
	else return;

	m_env.port = atoi(epos + 1);
}

void Connection::character(const char *charset)
{
	if (charset == 0) return;

	mysql_set_character_set(m_conn, charset);
	return;
}

Connection::Connection()
{
	m_conn = NULL;
	m_state = 0;

	memset(&m_env, 0, sizeof(LOGINENV));
	memset(&m_cda, 0, sizeof(m_cda));

	m_cda.rc = -1;
	strncpy(m_cda.message, "database not open.", 128);

	// 数据库种类
	memset(m_dbtype, 0, sizeof(m_dbtype));
	strcpy(m_dbtype, "mysql");
}

Connection::~Connection()
{
	disconnect();
}

int Connection::connecttodb(const char *connstr, 
							const char *charset, 
							unsigned int autocommitopt)
{
	// 若已连接上数据库，则不再连接
	if (m_state == 1) return 0;

	// 从connstr中解析username，password，tnsname
	setdbopt(connstr);

	memset(&m_cda, 0, sizeof(m_cda));

	// 分配或者初始化一个MYSQL对象，用于连接mysql服务端
	// 传入空指针会自动分配对象，需要注意mysql_close的调用
	if ( (m_conn = mysql_init(NULL)) == NULL )
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "initialize mysql failed.\n", 128);
		return -1;
	}

	// 连接数据库
	if ( mysql_real_connect(m_conn, m_env.ip, m_env.user, m_env.pwd,
			m_env.dbname, m_env.port, NULL, 0) == NULL)
	{
		m_cda.rc = mysql_errno(m_conn);
		strncpy(m_cda.message, mysql_error(m_conn), 2000);
		mysql_close(m_conn);
		m_conn = NULL;
		return -1;
	}

	// 设置事务模式，0-关闭自动提交，1-开启自动提交
	m_autocommitopt = autocommitopt;

	if (mysql_autocommit(m_conn, m_autocommitopt) != 0)
	{
		m_cda.rc = mysql_errno(m_conn);
		strncpy(m_cda.message, mysql_error(m_conn), 2000);
		mysql_close(m_conn);
		m_conn = NULL;
		return -1;
	}

	// 设置字符集
	character(charset);
	m_state = 1;

	// 设置事务隔离级别为 read committed
	execute("set session transaction ioslation level read committed");

	return 0;
}

int Connection::commit()
{
	memset(&m_cda, 0, sizeof(m_cda));

	if (m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.", 128);
		return -1;
	}

	if (mysql_commit(m_conn) != 0)
	{
		m_cda.rc = mysql_errno(m_conn);
		strncpy(m_cda.message, mysql_error(m_conn), 2000);
		mysql_close(m_conn);
		m_conn = NULL;
		return -1;
	}
	return 0;
}

int Connection::rollback()
{
	memset(&m_cda, 0, sizeof(m_cda));

	if (m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.", 128);
		return -1;
	}

	if (mysql_rollback(m_conn) != 0)
	{
		m_cda.rc = mysql_errno(m_conn);
		strncpy(m_cda.message, mysql_error(m_conn), 2000);
		mysql_close(m_conn);
		m_conn = NULL;
		return -1;
	}
	return 0;
}

int Connection::disconnect()
{
	memset(&m_cda, 0, sizeof(m_cda));

	if (m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.", 128);
		return -1;
	}

	rollback();
	mysql_close(m_conn);
	m_conn = NULL;
	m_state = 0;
	return 0;
}

int Connection::execute(const char *fmt, ...)
{
	memset(m_sql, 0, sizeof(m_sql));

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(m_sql, 10240, fmt, ap);
	va_end(ap);

	SqlStatement stmt(this);

	return stmt.execute(m_sql);
}

void Connection::err_report()
{
	if (m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.", 128);
		return;
	}

	memset(&m_cda, 0, sizeof(m_cda));

	m_cda.rc = -1;
	strncpy(m_cda.message, "call err_report failed.", 128);

	m_cda.rc = mysql_errno(m_conn);

	strncpy(m_cda.message, mysql_error(m_conn), 2000);

	return;
}

void SqlStatement::initial()
{
	m_state = 0;
	m_handle = NULL;

	memset(&m_cda, 0, sizeof(m_cda));
	memset(m_sql, 0, sizeof(m_sql));

	m_cda.rc = -1;
	strncpy(m_cda.message, "SqlStatement not connect to connection.\n", 128);
}

SqlStatement::SqlStatement()
{
	initial();
}

SqlStatement::SqlStatement(Connection *conn)
{
	initial();

	connect(conn);
}

SqlStatement::~SqlStatement()
{
	disconnect();
}

int SqlStatement::connect(Connection *conn)
{
	// 注意，一个SqlStatement在程序中只能绑定一个connection
	if (m_state == 1) return 0;

	memset(&m_cda, 0, sizeof(m_cda));
	m_conn = conn;

	// 如果数据库连接类的指针为空，直接返回失败
	if (m_conn == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.\n", 128);
		return -1;
	}

	// 如果数据库没有连接好，直接返回失败
	if (m_conn->m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "database not open.\n", 128);
		return -1;
	}

	// mysql_stmt结构初始化
	if ( (m_handle = mysql_stmt_init(m_conn->m_conn)) == NULL)
	{
		err_report();
		return m_cda.rc;
	}

	m_state = 1;

	m_autocommitopt = m_conn->m_autocommitopt;

	return 0;
}

int SqlStatement::disconnect()
{
	if (m_state == 0) return 0;
	memset(&m_cda, 0, sizeof(m_cda));

	mysql_stmt_close(m_handle);

	m_state = 0;
	m_handle = NULL;

	memset(&m_cda, 0, sizeof(m_cda));
	memset(&m_sql, 0, sizeof(m_sql));

	m_cda.rc = -1;
	strncpy(m_cda.message, "cursor not open.", 128);

	return 0;
}

void SqlStatement::err_report()
{
  // 注意，在该函数中，不可随意用memset(&m_cda,0,sizeof(m_cda))，否则会清空m_cda.rpc的内容
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return;
  }
  
  memset(&m_conn->m_cda,0,sizeof(m_conn->m_cda));

  m_cda.rc=-1;
  strncpy(m_cda.message,"call err_report() failed.\n",128);

  m_cda.rc=mysql_stmt_errno(m_handle);

  snprintf(m_cda.message,2000,"%d,%s",m_cda.rc,mysql_stmt_error(m_handle));

  m_conn->err_report();

  return;
}

// 把字符串中的小写字母转换成大写，忽略不是字母的字符。
// 这个函数只在prepare方法中用到。
void MY__ToUpper(char *str)
{
  if (str == 0) return;

  if (strlen(str) == 0) return;

  int istrlen=strlen(str);

  for (int ii=0;ii<istrlen;ii++)
  {
    if ( (str[ii] >= 'a') && (str[ii] <= 'z') ) str[ii]=str[ii] - 32;
  }
}

// 删除字符串左边的空格。
// 这个函数只在prepare方法中用到。
void MY__DeleteLChar(char *str,const char chr)
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

// 字符串替换函数
// 在字符串str中，如果存在字符串str1，就替换为字符串str2。
// 这个函数只在prepare方法中用到。
void MY__UpdateStr(char *str,const char *str1,const char *str2,bool bloop)
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
    strncpy(strTemp,str,strPos-str);
    strcat(strTemp,str2);
    strcat(strTemp,strPos+strlen(str1));
    strcpy(str,strTemp);

    strStart=strPos+strlen(str2);
  }
}


int SqlStatement::prepare(const char *fmt, ...)
{
	memset(&m_cda, 0, sizeof(m_cda));

	if (m_state == 0)
	{
		m_cda.rc = -1;
		strncpy(m_cda.message, "cursor not open.\n", 128);
		return -1;
	}

	memset(m_sql, 0, sizeof(m_sql));

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(m_sql, 10240, fmt, ap);
	va_end(ap);

	// 为了和oracle兼容，把:1,:2,:3...等替换成？
	char strcmp[11];
	for (int i = MAXPARAMS; i > 0; i--)
	{
		memset(strcmp, 0, sizeof(strcmp));
		snprintf(strcmp, 10, ":%d", i);
		MY__UpdateStr(m_sql, strcmp, "?", false);
	}

	// sysdate规制成sysdate()
	if (strstr(m_sql, "sysdate()") == 0)
		MY__UpdateStr(m_sql, "sysdate", "sysdate()", false);
	
	if (mysql_stmt_prepare(m_handle, m_sql, strlen(m_sql)) != 0)
	{
		err_report();
		memcpy(&m_cda1, &m_cda, sizeof(struct CDA_DEF));
		return m_cda.rc;
	}

	// 判断是否为查询语句，如果是，把m_sqltype设为0，其他语句设为1
	m_sqltype = 1;
	
	// 从待执行的SQL语句中截取30个字符，如果是以"select"打头，就认为是查询语句。
	char strtemp[31]; 
  memset(strtemp, 0, sizeof(strtemp)); 
  memcpy(strtemp, m_sql, sizeof(strtemp));
	MY__ToUpper(strtemp); MY__DeleteLChar(strtemp,' ');
	if (strncmp(strtemp,"SELECT",6)==0)  m_sqltype=0;

	memset(params_in,0,sizeof(params_in));

	memset(params_out,0,sizeof(params_out));

	maxbindin=0;

	return 0;
}

int SqlStatement::bindin(unsigned int position,int *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_LONG;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,long *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_LONGLONG;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,unsigned int *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_LONG;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,unsigned long *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_LONGLONG;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,char *value,unsigned int len)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_VAR_STRING;
  params_in[position-1].buffer = value;
  params_in[position-1].length=&params_in_length[position-1];
  params_in[position-1].is_null=&params_in_is_null[position-1];

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindinlob(unsigned int position,void *buffer,unsigned long *size)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_BLOB;
  params_in[position-1].buffer = buffer;
  params_in[position-1].length=size;
  params_in[position-1].is_null=&params_in_is_null[position-1];

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,float *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_FLOAT;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

int SqlStatement::bindin(unsigned int position,double *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->param_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_in[position-1].buffer_type = MYSQL_TYPE_DOUBLE;
  params_in[position-1].buffer = value;

  if (position>maxbindin) maxbindin=position;

  return 0;
}

///////////////////
int SqlStatement::bindout(unsigned int position,int *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_LONG;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::bindout(unsigned int position,long *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_LONGLONG;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::bindout(unsigned int position,unsigned int *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_LONG;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::bindout(unsigned int position,unsigned long *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_LONGLONG;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::bindout(unsigned int position,char *value,unsigned int len)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_VAR_STRING;
  params_out[position-1].buffer = value;
  params_out[position-1].buffer_length = len;

  return 0;
}

int SqlStatement::bindoutlob(unsigned int position,void *buffer,unsigned long buffersize,unsigned long *size)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_BLOB;
  params_out[position-1].length = size;
  params_out[position-1].buffer = buffer;
  params_out[position-1].buffer_length = buffersize;

  return 0;
}


int SqlStatement::bindout(unsigned int position,float *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_FLOAT;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::bindout(unsigned int position,double *value)
{
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (position<1) || (position>=MAXPARAMS) || (position>m_handle->field_count) )
  {
    m_cda.rc=-1; strncpy(m_cda.message,"array bound.",128);
  }

  params_out[position-1].buffer_type = MYSQL_TYPE_DOUBLE;
  params_out[position-1].buffer = value;

  return 0;
}

int SqlStatement::execute()
{
  memset(&m_cda,0,sizeof(m_cda));

  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }

  if ( (m_handle->param_count>0) && (m_handle->bind_param_done == 0))
  {
    if (mysql_stmt_bind_param(m_handle,params_in) != 0)
    {
      err_report(); return m_cda.rc;
    }
  }

  if ( (m_handle->field_count>0) && (m_handle->bind_result_done == 0) )
  {
    if (mysql_stmt_bind_result(m_handle,params_out) != 0)
    {
      err_report(); return m_cda.rc;
    }
  }
  
  // 处理字符串字段为空的情况。
  for (unsigned int ii=0;ii<maxbindin;ii++)
  {
    if (params_in[ii].buffer_type == MYSQL_TYPE_VAR_STRING )
    {
      if (strlen((char *)params_in[ii].buffer)==0) 
      {
        params_in_is_null[ii]=true;
      }
      else 
      {
        params_in_is_null[ii]=false;
        params_in_length[ii]=strlen((char *)params_in[ii].buffer);
      }
    }

    if (params_in[ii].buffer_type == MYSQL_TYPE_BLOB )
    {
      if ((*params_in[ii].length)==0) 
        params_in_is_null[ii]=true;
      else
        params_in_is_null[ii]=false;
    }
  }

  if (mysql_stmt_execute(m_handle) != 0)
  {
    err_report(); 

    if (m_cda.rc==1243) memcpy(&m_cda,&m_cda1,sizeof(struct CDA_DEF));

    return m_cda.rc;
  }
  
  // 如果不是查询语句，就获取影响记录的行数
  if (m_sqltype == 1) 
  { 
    m_cda.rpc=m_handle->affected_rows;
    m_conn->m_cda.rpc=m_cda.rpc;
  }

  /*
  if (m_sqltype == 0) 
   mysql_store_result(m_conn->m_conn);
  */
    
  return 0;
}

int SqlStatement::execute(const char *fmt,...)
{
  char strtmpsql[10241];
  memset(strtmpsql,0,sizeof(strtmpsql));

  va_list ap;
  va_start(ap,fmt);
  vsnprintf(strtmpsql,10240,fmt,ap);
  va_end(ap);

  if (prepare(strtmpsql) != 0) return m_cda.rc;

  return execute();
}

int SqlStatement::next()
{
  // 注意，在该函数中，不可随意用memset(&m_cda,0,sizeof(m_cda))，否则会清空m_cda.rpc的内容
  if (m_state == 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"cursor not open.\n",128); return -1;
  }
  
  // 如果语句未执行成功，直接返回失败。
  if (m_cda.rc != 0) return m_cda.rc;
  
  // 判断是否是查询语句，如果不是，直接返回错误
  if (m_sqltype != 0)
  {
    m_cda.rc=-1; strncpy(m_cda.message,"no recordset found.\n",128); return -1;
  }
  
  int ret=mysql_stmt_fetch(m_handle);

  if (ret==0) 
  { 
    m_cda.rpc++; return 0; 
  }
 
  if (ret==1) 
  {
    err_report(); return m_cda.rc;
  }

  if (ret==MYSQL_NO_DATA) return MYSQL_NO_DATA;

  if (ret==MYSQL_DATA_TRUNCATED) 
  {
    m_cda.rpc++; return 0;
  }
  
  return 0;
}

// 把文件filename加载到buffer中，必须确保buffer足够大。
// 成功返回文件的大小，文件不存在或为空返回0。
unsigned long filetobuf(const char *filename,char *buffer)
{
  FILE *fp;
  int  bytes=0;
  int  total_bytes=0;

  if ( (fp=fopen(filename,"r")) ==0 ) return 0;

  while (true)
  {
    bytes=fread(buffer+total_bytes,1,5000,fp);

    total_bytes = total_bytes + bytes;

    if (bytes<5000) break;
  }

  fclose(fp);

  return total_bytes;
}

// 把buffer中的内容写入文件filename，size为buffer中有效内容的大小。      
// 成功返回true，失败返回false。
bool buftofile(const char *filename,char *buffer,unsigned long size)
{
  if (size==0) return false;

  char filenametmp[301];
  memset(filenametmp,0,sizeof(filenametmp));
  snprintf(filenametmp,300,"%s.tmp",filename);

  FILE *fp;

  if ( (fp=fopen(filenametmp,"w")) ==0 ) return false;

  // 如果buffer比较大，可能存在一次write不完的情况，以下代码可以优化成用一个循环写入。
  size_t tt=fwrite(buffer,1,size,fp);

  if (tt!=size)
  {
    remove(filenametmp); return false;
  }

  fclose(fp);

  if (rename(filenametmp,filename) != 0) return false;

  return true;
}