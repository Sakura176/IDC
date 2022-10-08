#include "../include/crtdata.h"

int main(int argc, char const *argv[])
{
	CrtData crtData("/home/yc/IDC/data/ini/stcode.ini", "/home/yc/IDC/data/surfdata");
	crtData.run();
	crtData.writeToFile("json");
	crtData.writeToFile("csv");
	crtData.writeToFile("xml");
	return 0;
}
