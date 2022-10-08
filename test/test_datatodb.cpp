#include "../include/_mysql.h"
#include "../include/obttodb.h"

int main(int argc, char const *argv[])
{
	ObtToDB obtToDB("/home/yc/IDC/data/ini/stcode.ini", "/home/yc/IDC/data/surfdata");
	obtToDB.codeToDB();
	obtToDB.dataToDB();

	return 0;
}
