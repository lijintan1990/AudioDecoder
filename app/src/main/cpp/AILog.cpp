#include "AILog.h"
#include <time.h>
#include <stdarg.h>

#ifdef ANDROID
#include <sys/stat.h>
#include <cstdlib>

#endif

#define LOG_FILE_SIZE           (500000)
#define LOG_FILE_SIZE_APPEND    (10000)


shared_ptr<AILog> AILog::mLog(new AILog());
vector<string> AILog::mLevelName = { "ERROR", "WARNING", "INFO", "DEBUG" };

AILog::AILog() {
	mFilePtr = nullptr;
	mLogLevel = AIERROR;
}

AILog::~AILog()
{
	if (mFilePtr)
	{
		fclose(mFilePtr);
		mFilePtr = nullptr;
	}
}

shared_ptr<AILog> AILog::getInstance()
{
	return mLog;
}

void AILog::setLogPath(string path)
{
	mLogFilePath = path;
}

void AILog::setLogLevel(LogLevel level)
{
	mLogLevel = level;
}

void AILog::outPut(const char *fileName, const char* funName, int line, LogLevel level, const char *strFmt, ...)
{
	if (level > mLogLevel)
	{
		return;
	}

	char targetText[512] = { 0 };
	time_t t;
	struct tm *mt;
	time(&t);
	mt = localtime(&t);
	char strDate[64] = { 0 };
	char strTime[64] = { 0 };
	char strContent[256] = { 0 };
	strftime(strDate, sizeof(strDate), "%Y-%m-%d", mt);
	strftime(strTime, sizeof(strTime), "%H:%M:%S", mt);

	va_list v;
	va_start(v, strFmt);
	vsnprintf(strContent, sizeof(strContent)-1, strFmt, v);
	va_end(v);
	char subFileName[16] = { 0 };
	memcpy(subFileName, fileName + strlen(fileName) - 15, 15);
	sprintf(targetText, "[%s %s] [%s:%s:%d] %s %s\n", strDate, strTime, subFileName, funName, line, mLevelName[level].c_str(), strContent);
	AIPrintf(targetText, level);
	writeFile(targetText);
}

#ifdef ANDROID
void AILog::androidPrint(const char *str, LogLevel level)
{
	switch (level){
	case AIERROR:
		__android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", str);
		break;
	case AIWARNING:
		__android_log_print(ANDROID_LOG_WARN, "AINative", "%s", str);
		break;
	case AIINFO:
		__android_log_print(ANDROID_LOG_INFO, "AINative", "%s", str);
		break;
	case AIDEBUG:
		__android_log_print(ANDROID_LOG_DEBUG, "AINative", "%s", str);
		break;
	}
}
#endif

void AILog::AIPrintf(char *str, LogLevel level)
{
#ifdef ANDROID
	androidPrint(str, level);
#elif _WIN32
	WCHAR wszBuf[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wszBuf, sizeof(wszBuf));
	OutputDebugStringW(wszBuf);
	WideCharToMultiByte(CP_ACP, 0, wszBuf, -1, str, sizeof(str), nullptr, FALSE);
#else
	printf("%s", str);
#endif
}

void AILog::writeFile(const char *str)
{
	if (mLogFilePath == "")
	{
		return;
	}

	LazyUtility::CLazyLock lock(&mlock);

	if (mFilePtr == nullptr) {
		mFilePtr = fopen(mLogFilePath.c_str(), "a+");

		if (nullptr == mFilePtr) {
			return;
		}

#ifndef _WIN32
		//        chmod(mLogFilePath.c_str(), 00777);
		fseek(mFilePtr, 0, SEEK_END);
		int nFileLen = ftell(mFilePtr);
		char* pData = (char *)calloc(1, nFileLen);
		fseek(mFilePtr, 0, SEEK_SET);
		fread(pData, nFileLen, 1, mFilePtr);

		if (nFileLen > LOG_FILE_SIZE) {
			fclose(mFilePtr);
			mFilePtr = NULL;
			mFilePtr = fopen(mLogFilePath.c_str(), "wb+");
			fwrite(pData + nFileLen - LOG_FILE_SIZE_APPEND, LOG_FILE_SIZE_APPEND, 1, mFilePtr);
		}

		free(pData);
#endif
	}

	fwrite(str, strlen(str), 1, mFilePtr);
	fflush(mFilePtr);
}
