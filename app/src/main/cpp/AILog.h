#ifndef __AI_LOG_H__
#define __AI_LOG_H__

#include <string>
#include <vector>
#include <memory>
#include "LazyLock.h"
#ifdef ANDROID
#include <android/log.h>
#endif
using namespace std;

#define     LOGD(FMT, ...)	AILog::getInstance()->outPut(__FILE__, __FUNCTION__, __LINE__, AILog::LogLevel::AIDEBUG, FMT, ##__VA_ARGS__)
#define     LOGI(FMT, ...)	AILog::getInstance()->outPut(__FILE__, __FUNCTION__, __LINE__, AILog::LogLevel::AIINFO, FMT, ##__VA_ARGS__)
#define     LOGW(FMT, ...)	AILog::getInstance()->outPut(__FILE__, __FUNCTION__, __LINE__, AILog::LogLevel::AIWARNING, FMT, ##__VA_ARGS__)
#define     LOGE(FMT, ...)	AILog::getInstance()->outPut(__FILE__, __FUNCTION__, __LINE__, AILog::LogLevel::AIERROR, FMT, ##__VA_ARGS__)

class AILog
{
public:
	enum LogLevel
	{
		AIERROR = 0,
		AIWARNING,
		AIINFO,
		AIDEBUG,
	};

	~AILog();
	static shared_ptr<AILog> getInstance();
	void setLogPath(string path);
	void setLogLevel(LogLevel level);
	void outPut(const char* fileName, const char* funName, int line, LogLevel level, const char *strFmt, ...);
private:
	AILog();
	void writeFile(const char *str);
	void AIPrintf(char *str, LogLevel level);
#ifdef ANDROID
	void androidPrint(const char *str, LogLevel level);
#endif

private:
	static shared_ptr<AILog> mLog;
	static vector<string> mLevelName;

	FILE* mFilePtr;
	LogLevel mLogLevel;
	string mLogFilePath;
	LazyUtility::CLazyCriSec mlock;
};

#endif // __AI_LOG_H__