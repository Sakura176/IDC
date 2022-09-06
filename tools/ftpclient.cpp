#include "_ftp.h"

Cftp ftp;

int main(int argc, char const *argv[])
{
	if (ftp.login("47.98.208.80:21", "yc", "436052") == false)
	{
		printf("ftp.login(47.98.208.80:21) failed.\n"); return -1;
	}

	printf("ftp.login(47.98.208.80:21) ok.\n");

	if (ftp.mtime("/project/public/_public.cpp") == false)
	{
		printf("ftp.mtime(/project/public/_public.cpp) failed.\n"); return -1;
	}

	printf("ftp.mtime(/project/public/_public.cpp) ok, mtime=%s.\n", ftp.m_mtime);

	if (ftp.size("/project/public/_public.cpp") == false)
	{
		printf("ftp.size(/project/public/_public.cpp) failed.\n"); return -1;
	}
	printf("ftp.size(/project/public/_public.cpp) ok, size=%d.\n", ftp.m_size);

	if (ftp.nlist("/project/tmp", "/project/demo/test/file_list.lst") == false)
	{
		printf("ftp.nlist(/project/tmp) failed.\n"); 
	}
	else
		printf("ftp.nlist(/project/tmp) ok.\n"); 

	if (ftp.get("/project/tmp/libftp.a", "/project/demo/test/libftp.a.bak") == false)
	{
		printf("ftp.get(/project/tmp/libftp.a) failed.\n");
	}
	else
		printf("ftp.get(/project/tmp/libftp.a) ok.\n");

	if (ftp.put("/project/demo/test/libftp.a.bak", "/project/tmp/libftp.a.bak") == false)
		printf("ftp.put(/project/demo/test/libftp.a.bak) failed.\n");
	else
		printf("ftp.put(/project/demo/test/libftp.a.bak) ok.\n");
	
	if (ftp.chdir("/project/demo/test") == false)
		printf("ftp.chdir(/project/demo/test) failed.\n");
	else
		printf("ftp.chdir(/project/demo/test) ok.\n");

	ftp.logout();
	return 0;
}
