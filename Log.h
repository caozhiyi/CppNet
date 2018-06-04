#ifndef HEADER_LOG
#define HEADER_LOG

#include <fstream>
#include <stdarg.h>

#include "RunnableAloneTaskList.h"
#include "MemaryPool.h"
#include "Time.h"
#include "Single.h"

enum LogLevel {
	LOG_DEBUG_LEVEL		= 0x0001,
	LOG_INFO_LEVEL		= 0x0003,
	LOG_WARN_LEVEL		= 0x0007,
	LOG_ERROR_LEVEL		= 0x000F,
	LOG_FATAL_LEVEL		= 0x001F,
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

	void LogDebug(const char* file, int line, const char* log...);
	void LogInfo(const char* file, int line, const char* log...);
	void LogWarn(const char* file, int line, const char* log...);
	void LogError(const char* file, int line, const char* log...);
	void LogFatal(const char* file, int line, const char* log...);

private:
	void _PushFormatLog(const char* file, int line, char* level, const char* log, va_list list);
	void _CheckDateFile();

private:
	CTime			_time;
	std::string     _file_name;
	std::fstream	_log_file;
	int				_log_level;
	int				_cur_date;
	CMemaryPool		_pool;
};

#define LOG_DEBUG(log, ...)			CLog::Instance().LogDebug(__FILE__, __LINE__, log, __VA_ARGS__);
#define LOG_INFO(log, ...)			CLog::Instance().LogInfo(__FILE__, __LINE__, log, __VA_ARGS__);
#define LOG_WARN(log, ...)			CLog::Instance().LogWarn(__FILE__, __LINE__, log, __VA_ARGS__);
#define LOG_ERROR(log, ...)			CLog::Instance().LogError(__FILE__, __LINE__, log, __VA_ARGS__);
#define LOG_FATAL(log, ...)			CLog::Instance().LogFatal(__FILE__, __LINE__, log, __VA_ARGS__);

#endif