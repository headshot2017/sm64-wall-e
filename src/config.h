#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define CONFIG_FILENAME "../sm64-wall-e.cfg"

#include <string>
#include <map>

struct ConfigElement
{
	const std::string desc;
	int value;
};

extern std::map<std::string, ConfigElement> config;

int getConfig(std::string value);
void saveConfig();
void loadConfig();

#endif // CONFIG_H_INCLUDED
