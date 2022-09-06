#include "../src/idc/crtdata.h"

int main(int argc, char const *argv[])
{
	CrtData crtData("../data/ini/stcode.ini", "../data/surfdata");
	crtData.run();
	crtData.writeToFile("json");
	return 0;
}
