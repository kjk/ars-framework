#ifndef __ARSLEXIS_LOGGING_HPP__
#define __ARSLEXIS_LOGGING_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <DynStr.hpp>

enum LogLevel 
{
    eLogAlways,
    eLogCritical=10,
    eLogError=20,
    eLogDebug=30,
    eLogWarning=40,
    eLogInfo=50,
    eLogEverything = 500,  // a big number
    eLogLevelDefault=eLogDebug,
};

void InitLogging();
void DeinitLogging();

void LogAddMemoLog(LogLevel threshold);
void LogAddDebuggerLog(LogLevel threshold);
void LogAddFileLog(const char_t *fileName, LogLevel threshold);

void Log(LogLevel level, const char_t *txt, bool fFinishLine);
void Log(LogLevel level, const char_t *txt, ulong_t len, bool fFinishLine);
void LogUlong(LogLevel level, unsigned long num, bool fFinishLine);
void LogStrUlong(LogLevel level, const char_t *txt, unsigned long num);

#endif
