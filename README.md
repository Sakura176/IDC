<!--
 * @brief: 
 * @Autor: yc
 * @Date: 2022-07-12 09:52:30
 * @LastEditors: yc
 * @LastEditTime: 2022-07-21 16:07:48
 * @FilePath: /project/README.md
-->
# 气象数据中心

## 服务程序框架
### 气象数据生成：模拟生成气象数据，包含csv，json，xml三种格式的数据
+ 读取全国气象站点数据，使用随机数对每个站点生成数据以模拟气象数据中心
+ 将生成的数据写入文件，支持csv，json，xml三种格式
### 服务程序调用程序：后台调用气象数据生成等程序，并定时检查
+ 该程序用于后台启动其他程序，包括气象数据生成程序
+ 思路：在程序中创建子进程并让父进程退出，通过让系统进程托管子进程的方法达到后台运行的效果。
### 文件压缩和删除程序：对早期文件进行压缩和删除
+ 文件压缩程序可以对设定时间之前的文件进行压缩，包括日志文件
+ 删除程序可删除时间太久的数据
### 后台服务程序检测程序
+ 检测后台服务程序是否超时，通过心跳程序实现，后台程序调用心跳程序类往共享内存中添加心跳信息，超时则杀死程序。

## FTP文件传输系统

## TCP文件传输系统

### TCP长连接
1. 介绍：指在一个TCP连接上可以连续发送多个数据包。在TCP连接保持期间，如果没有数据包发送，需**双方发送探测包以维持此连接**，一般需要自己做在线维持（不发送RST包和四次挥手）。
2. 过程：
   ```C++
   连接→数据传输→保持连接(心跳)→数据传输→保持连接(心跳)→……→关闭连接（一个TCP连接通道多个读写通信）。 这就要求长连接在没有数据通信时，定时发送数据包(心跳)，以维持连接状态；
   ```
3. 应用场景：
   数据库，各种组件的连接等；长连接多用于操作频繁（读写），点对点的通讯，且连接数不能太多的场景。因为频繁建立tcp连接，是对资源的浪费，服务器的请求处理速度也会更低。
4. 优缺点：
   + 优点：长连接可以省去较多的TCP建立和关闭的操作（三次握手/四次挥手），减少资源浪费，节约连接时间。对于频繁请求资源的客户来说，更适用长连接。
   + 缺点：client与server间的连接如果一直不关闭，会存在一个问题：随着客户端连接越来越多，server早晚有扛不住的时候
### TCP短连接
1. 介绍：
	通信双方有数据交互时，就建立一个TCP连接，数据发送完成后，则断开此TCP连接（管理起来比较简单，存在的连接都是有用的连接，不需要额外的控制手段）；
2. 应用场景：
   WEB网站的http服务一般都用短链接（http1.0默认为短连接，而http1.1之后默认为长连接，keep-alive 带时间，操作次数限制的长连接），因为长连接对于服务端来说会耗费一定的资源，而像WEB网站这么频繁的成千上万甚至上亿客户端的连接用短连接会更省一些资源（也就是正常情况下，每个用户不会频繁发送请求，没必要长时间占用一条连接或端口）
3. 优点：
   短连接对于服务器来说管理较为简单，存在的连接都是有用的连接，不需要额外的控制手段
4. 缺点：
   如果客户请求频繁，将在TCP的建立和关闭操作上浪费时间和带宽，消耗服务端的资源

### TCP文件传输实现逻辑
#### 客户端文件发送
1. 连接服务器
2. 发送登录信息
3. 检索文件目录，记录待发送文件清单，准备逐个发送
4. 读取文件名、时间和大小并发送
5. 逐个发送文件内容，每个文件内容发送完毕则发送一条结束信息
#### 服务端文件接收
1. 启动服务器等待连接
2. 接收登录信息并回复
3. 接收文件名等信息并回复确认消息）
4. 接收文件结束信息，结束文件接收，并继续等待信息
#### 缺陷
+ 如果文件传输一半程序中断该如何处理？
  1. 使用临时文件名，文件传输完成后再改名
+ 传输过程中客户端文件流一直处于打开状态，是否存在影响？
+ 文件上传后本地文件的处理方法？
#### 异步通讯
+ 多进程：用不同的进程发送报文和接收报文
+ 多线程：用不同的线程发送报文和接收报文
+ I/O复用：selct、poll、epoll函数
#### 代码改进目标
+ [x] 使用临时文件名接收文件，接收完成后更名
+ [x] 文件上传后根据输入参数删除或备份本地文件
+ [ ] 使用类封装文件上传过程
+ [ ] 异步通讯实现文件的上传和下载

### Linux共享内存
1. 调用shmget函数获取或创建共享内存
```c++
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
```
2. 调用shmat函数把共享内存连接到当前进程的地址空间
```C++
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
```
3. 调用shmdt函数把共享内存从当前进程中分离
```c++
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
```
4. 调用shmctl函数删除共享内存
```c++
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```
案例
```C++
struct st_pid
{
	int pid;		// 进程编号。
	char name[51];	// 进程名称。
};


int main(int argc, char const *argv[])
{
	// 共享内存的标志
	int shmid;
	
	// 获取或者创建共享内存，键值为0x5005
	if ( (shmid=shmget(0x5005, sizeof(struct st_pid), 0640|IPC_CREAT)) == -1)
	{
		printf("shmget(0x5005) failed\n"); return -1;
	}

	// 用于指向共享内存的结构体变量
	struct st_pid *stpid=0;

	// 把共享内存连接到当前进程的地址空间
	if( (stpid=(struct st_pid *)shmat(shmid, 0, 0)) == (void *)-1)
	{
		printf("shmat failed\n"); return -1;
	}

	printf("pid=%d, name=%s\n", stpid->pid, stpid->name);
	stpid->pid = getpid();
	strcpy(stpid->name, argv[1]);

	printf("pid=%d, name=%s\n", stpid->pid, stpid->name);

	// 把共享内存从当前进程中分离
	shmdt(stpid);

	// if (shmctl(shmid, IPC_RMID, 0)==-1)
	// {
	// 	printf("shmctl failed\n");
	// 	return -1;
	// }

	return 0;
}
```

```bash
ipcs -m  # 查看共享内存
ipcrm -m [shmid] # 删除共享内存
```

### Linxu信号量
+ 信号量本质上是一个非负数的计数器
+ 给共享资源建立一个标志，表示该共享资源被占用情况
+ P操作(申请资源-1)，V操作(释放资源+1)  

1. 信号量创建函数
```c++
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/**
 * @brief 
 * @param key 信号量的键
 * @param nsems 信号量的个数
 * @param semflg 操作类型以及操纵权限
 * @return 如果成功，则返回信号量集的IPC标识符。如果失败，则返回 -1。同时设置errno变量以说明错误原因。
*/
int semget(key_t key, int nsems, int semflg);
```

2. 信号量操作函数
```c++
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/**
 * @brief 
 * @param simid 信号集的识别码，可通过semget获取
 * @param nsems 指向存储信号量结构体的数组指针
 * @param nsops 信号量结构体的数量，恒大于或等于1
 * @return 如果成功，则返回0。如果失败，则返回 -1。同时设置errno变量以说明错误原因。
*/
int semop(int semid, struct sembuf *sops, size_t nsops);

struct sembuf
{
  unsigned short int sem_num;	/* semaphore number */
  short int sem_op;		/* semaphore operation */
  short int sem_flg;		/* operation flag */
};
```

3. 信号量控制函数
```c++
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/**
 * @brief 
 * @param simid 信号集的识别码，可通过semget获取
 * @param semnum 信号集的索引，用于获取信号集内的某个信号
 * @param cmd 需要执行的命令
 * @return 成功执行时，根据不同的命令返回不同的非负值。如果失败，则返回 -1。同时设置errno变量以说明错误原因。
*/
int semctl(int semid, int semnum, int cmd, ...);

```

### 守护进程
#### 服务程序在共享内存中维护自己的心跳信息
#### 开发守护程序，终止已经死机的服务程序

```bash
ipcs -m			# 查看共享内存
ipcrm -m shmid	# 删除共享内存
ipcs -s			# 查看信号量
ipcrm sem semid # 删除信号量
```

## GDB调试技巧

```gdb
run				// 运行代码
b 代码行数		// 添加断点
p 变量名		// 打印变量内容
set args		// 添加输入参数
```
```C++
build
bin
lib
src
  - log
  - mysql
  - public
  - socket
  - idc
tmp
ini
tools
test
```