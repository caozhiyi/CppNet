#include <iostream>

#include "Log.h"

const static int __log_buf_size = 1024;

CLog::CLog() : _log_level(LOG_ERROR_LEVEL), _cur_date(0), _pool(__log_buf_size, 40){

}

CLog::~CLog() {
    Stop();
    Join();
	_log_file.close();
}

void CLog::Run() {
	while (!_stop) {
		char* log = _Pop();
		if (log) {
			_CheckDateFile();
			if (_log_file.is_open()) {
				std::cout << log << std::endl;
				_log_file << log << std::endl;
			}
			_pool.PoolLargeFree(log);
		} else {
			break;
		}
	}
}

void CLog::Stop() {
	_stop = true;
	Push(nullptr);
}

void CLog::SetLogName(const std::string& file_name) {
	_file_name = file_name;
}

std::string CLog::GetLogName() {
	return _file_name;
}

void CLog::SetLogLevel(LogLevel level) {
	_log_level = level;
}

LogLevel CLog::GetLogLevel() {
	return (LogLevel)_log_level;
}

void CLog::LogDebug(const char* file, int line, const char* log...) {
	if (_log_level <= LOG_DEBUG_LEVEL) {
		va_list list;
		va_start(list, log);
		_PushFormatLog(file, line, "DEBUG", log, list);
		va_end(list);
	}
}

void CLog::LogInfo(const char* file, int line, const char* log...) {
	if (_log_level <= LOG_INFO_LEVEL) {
		va_list list;
		va_start(list, log);
		_PushFormatLog(file, line, "INFO", log, list);
		va_end(list);
	}
}

void CLog::LogWarn(const char* file, int line, const char* log...) {
	if (_log_level <= LOG_WARN_LEVEL) {
		va_list list;
		va_start(list, log);
		_PushFormatLog(file, line, "WARN", log, list);
		va_end(list);
	}
}

void CLog::LogError(const char* file, int line, const char* log...) {
	if (_log_level <= LOG_ERROR_LEVEL) {
		va_list list;
		va_start(list, log);
		_PushFormatLog(file, line, "ERROR", log, list);
		va_end(list);
	}
}

void CLog::LogFatal(const char* file, int line, const char* log...) {
	if (_log_level <= LOG_FATAL_LEVEL) {
		va_list list;
		va_start(list, log);
		_PushFormatLog(file, line, "FATAL", log, list);
		va_end(list);
	}
}

void CLog::_PushFormatLog(const char* file, int line, const char* level, const char* log, va_list list) {
	_time.Now();

	char* time = _pool.PoolMalloc<char>(32);
	_time.GetFormatTime(time, 32);
	char* log_str =_pool.PoolLargeMalloc<char>();
	int curlen = snprintf(log_str, __log_buf_size, "[%s:%s-%s:%d] ", time, level, file, line);
	_pool.PoolFree(time, 32);

	if (curlen < 0) {
		strcpy(log_str, "...Log format error!");

	} else {
		vsnprintf(log_str + curlen, __log_buf_size - curlen, log, list);
	}
	Push(log_str);
}

void CLog::_CheckDateFile() {
	if (_cur_date != _time.GetDate()) {
		_cur_date = _time.GetDate();
		if (_log_file.is_open()) {
			_log_file.close();
		}

		std::string file_name = _file_name.append(".");
		file_name.append(_time.GetDateStr());
		file_name.append(".log");

		_log_file.open(file_name.c_str(), std::ios::app | std::ios::out);
	}
}