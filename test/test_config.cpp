#include "../include/config.h"
#include "../include/dbtofile.h"

int main(int argc, char const *argv[])
{
	// YAML::Node root = YAML::LoadFile("/home/yc/IDC/conf/dbtofile.yaml");
	// server::Config::LoadFromYaml(root);
	// std::cout << root << std::endl;
	DBToFile dbToFile("/home/yc/IDC/conf/dbtofile.yaml");
	dbToFile.run();
	// dbToFile.writeToFile();
	return 0;
}
