#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#include <string>
#include <map>
#include <mutex>

class CConfig
{
public:
	bool SetFilePath(const std::string& path);
	bool ReLoadFile();

	bool LoadFile(const std::string& path);
	
	int GetIntValue(const std::string& key);
	std::string GetStringValue(const std::string& key);
	double GetDoubleValue(const std::string& key);
	bool GetBoolValue(const std::string& key);

private:
	void _Trim(std::string& line);

private:
	std::string _file;
	std::mutex	_mutex;
	std::map<std::string, std::string> _config_map;
};

#endif