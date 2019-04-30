#ifndef HEADER_LOG
#define HEADER_LOG

#include <fstream>
#include <stdarg.h>

#include "RunnableAloneTaskList.h"
#include "MemaryPool.h"
#include "TimeTool.h"
#include "Single.h"

enum LogLevel {
	LOG_DEBUG_LEVEL		= 1,
	LOG_INFO_LEVEL		= 2,
	LOG_WARN_LEVEL		= 3,
	LOG_ERROR_LEVEL		= 4,
	LOG_FATAL_LEVEL		= 5,
};

class CLog: public CRunnableAloneTaskList<char*>, public CSingle<CLog>
{
public:
	CLog();
	virtual ~CLog();
	virtual void Run();
	virtual void Stop();
	void SetLogName(const std::string& file_name);
	std::string GetLogName();

	void SetLogLevel(LogLevel level);
	LogLevel GetLogLevel();

	//api for different level log
	void LogDebug(const char* file, int line, const char* log...);
	void LogInfo(const char* file, int line, const char* log...);
	void LogWarn(const char* file, int line, const char* log...);
	void LogError(const char* file, int line, const char* log...);
	void LogFatal(const char* file, int line, const char* log...);

private:
	//format log and push to task queue
	void _PushFormatLog(const char* file, int line, const char* level, const char* log, va_list list);
	//check date and create new log file
	void _CheckDateFile();

private:
	CTimeTool		_time;			//for now tile
	std::string     _file_name;
	std::fstream	_log_file;
	int				_log_level;
	int				_cur_date;
	CMemoryPool		_pool;
};

#define LOG_DEBUG(log, ...)			CLog::Instance().LogDebug(__FILE__, __LINE__, log, ##__VA_ARGS__);
#define LOG_INFO(log, ...)			CLog::Instance().LogInfo(__FILE__, __LINE__, log, ##__VA_ARGS__);
#define LOG_WARN(log, ...)			CLog::Instance().LogWarn(__FILE__, __LINE__, log, ##__VA_ARGS__);
#define LOG_ERROR(log, ...)			CLog::Instance().LogError(__FILE__, __LINE__, log, ##__VA_ARGS__);
#define LOG_FATAL(log, ...)			CLog::Instance().LogFatal(__FILE__, __LINE__, log, ##__VA_ARGS__);

#endif