#ifndef __ZYLOG_H_
#define __ZYLOG_H_

#include <string>

#define LOG_DEBUG(fmt, args...) (CZYLog::Instance()->WriteLog(__FILE__, __LINE__, __FUNCTION__, "DEBUG", fmt, ##args))
// #define LOG_INFO(fmt, args...) 
// #define	LOG_WARN(fmt, args...) 
#define LOG_ERROR(fmt, args...) (CZYLog::Instance()->WriteLog(__FILE__, __LINE__, __FUNCTION__, "ERROR", fmt, ##args))


class CZYLog
{
public:
	static CZYLog* Instance();

	void WriteLog(const char *filename, int line, const char *funcname, const char *level, const char *msg, ...);

private:
	CZYLog() {}

private:
	std::string m_strLogDir;		// log dir
	std::string m_strFileName;		// file name
};

#endif /* __ZYLOG_H_ */
