#ifdef _PALM_OS
#include <Application.hpp>
#endif

#include <Logging.hpp>

#define MAX_LOGGERS_COUNT 3

enum LoggerType {
    loggerTypeMemo,
    loggerTypeFile,
    loggerTypeDebugger
};

typedef struct LoggerInfoTag {
    LoggerType      type;
    // logging threshold for this logger
    // log everything above this threshold
    LogLevel        threshold;

    // only for loggerTypeMemo
    DmOpenRef       db;

    // fileName and file are only for loggerTypeFile
    const char_t *  fileName; 
    HostFILEType *  file;
} LoggerInfo;

typedef struct LoggingGlobalsTag {
    int             loggersCount;
    LoggerInfo      loggers[MAX_LOGGERS_COUNT];
    DynStr          curLine;
} LoggingGlobals;

// using globals so not safe for non-global situations. just be careful.
// could use a feature to be global-safe
LoggingGlobals *g_logging = NULL;

static LoggingGlobals *GetLoggingGlobals(bool fCreateIfNotExists)
{
    if (NULL != g_logging)
        return g_logging;

    if (!fCreateIfNotExists)
        return NULL;

    g_logging = (LoggingGlobals*)malloc(sizeof(LoggingGlobals));
    if (NULL == g_logging)
        return NULL;
    g_logging->loggersCount = 0;
    if (NULL == DynStrInit(&g_logging->curLine, 128))
    {
        free(g_logging);
        g_logging = NULL;
        return NULL;
    }
    return g_logging;
}

void InitLogging()
{
    // yes, nothing to do. we allocate g_logging lazily
}

void DeinitLogging()
{
    LoggingGlobals *lg = GetLoggingGlobals(true);
    if (NULL == lg)
        return;

    for (int i=0; i<lg->loggersCount; i++)
    {
#ifdef _PALM_OS
        if (loggerTypeFile == lg->loggers[i].type)
        {
            if (NULL != lg->loggers[i].file)
            {
                HostFFlush(lg->loggers[i].file);
                HostFClose(lg->loggers[i].file);
            }
        }
#endif
        if (loggerTypeMemo == lg->loggers[i].type)
        {

            if (NULL != lg->loggers[i].db)
            {
                DmCloseDatabase(lg->loggers[i].db);
                lg->loggers[i].db = NULL;
            }
        }

        if (loggerTypeDebugger == lg->loggers[i].type)
        {
            // do nothing
        }
    }

    DynStrFree(&lg->curLine);

    if (NULL != lg)
        free(lg);
}

void LogAddMemoLog(LogLevel threshold)
{
#if defined(_PALM_OS)
    LoggingGlobals *lg = GetLoggingGlobals(true);
    if (NULL == lg)
        return;
    if (lg->loggersCount >= MAX_LOGGERS_COUNT)
    {
        assert(false);
        return;
    }
    LoggerInfo *loggerInfo = &(lg->loggers[lg->loggersCount]);

    loggerInfo->db = DmOpenDatabaseByTypeCreator('DATA', 'memo', dmModeReadWrite);
    if (NULL == loggerInfo->db)
        return;
    loggerInfo->type = loggerTypeMemo;
    loggerInfo->threshold = threshold;
    ++lg->loggersCount;
#else
    // only available for Palm
    assert(false);
#endif
}

void LogAddDebuggerLog(LogLevel threshold)
{
    LoggingGlobals *lg = GetLoggingGlobals(true);
    if (NULL == lg)
        return;
    if (lg->loggersCount >= MAX_LOGGERS_COUNT)
    {
        assert(false);
        return;
    }
    LoggerInfo *loggerInfo = &(lg->loggers[lg->loggersCount]);
    loggerInfo->type = loggerTypeDebugger;
    loggerInfo->threshold = threshold;
    ++lg->loggersCount;
}

void LogAddFileLog(const char_t *fileName, LogLevel threshold)
{
#if defined(_PALM_OS)
    LoggingGlobals *lg = GetLoggingGlobals(true);
    if (NULL == lg)
        return;
    if (lg->loggersCount >= MAX_LOGGERS_COUNT)
    {
        assert(false);
        return;
    }
    LoggerInfo *loggerInfo = &(lg->loggers[lg->loggersCount]);

    loggerInfo->file = HostFOpen(fileName, "w");
    if (NULL == loggerInfo->file)
        return;

    loggerInfo->type = loggerTypeFile;
    loggerInfo->fileName = fileName;
    loggerInfo->threshold = threshold;
    ++lg->loggersCount;
#else
        // only available for Palm
        assert(false);
#endif
}

// return true if logging threshold of any of the loggers is lower than
// requested level of logging i.e. true if we need to log a give level
static bool FNeedsLogging(LoggingGlobals *lg, LogLevel level)
{
    for (int i=0; i<lg->loggersCount; i++)
    {
        if (lg->loggers[i].threshold >= level)
            return true;
    }
    return false;
}

static void AddToLog(DynStr *log, const char_t *txt, bool fFirst)
{
    if (fFirst)
    {
        char_t buffer[12] = {0};
        tick_t timestamp = ticks();
        tprintf(buffer, _T("%08lx "), timestamp);
        DynStrAppendCharP(log, buffer);
    }

    // don't care if fails to append
    DynStrAppendCharP(log, txt);
}

static void LogRaw(LoggingGlobals *lg, LogLevel level, char_t *txt)
{
    for (int i=0; i<lg->loggersCount; i++)
    {
        if (lg->loggers[i].threshold >= level)
        {
            if (loggerTypeMemo == lg->loggers[i].type)
            {
#if defined(_PALM_OS)
                if (0 != lg->loggers[i].db)
                {
                    // only log if we were able to open a database
                    UInt16 index = dmMaxRecordIndex;
                    UInt16 len = tstrlen(txt)+1;
                    MemHandle handle = DmNewRecord(lg->loggers[i].db, &index, len);
                    if (!handle)
                    {
                        DmCloseDatabase(lg->loggers[i].db);
                        lg->loggers[i].db = NULL;
                        return;
                    }
                    void* data = MemHandleLock(handle);
                    if (data)
                    {
                        DmWrite(data, 0, txt, len);
                        MemHandleUnlock(handle);
                    }
                    DmReleaseRecord(lg->loggers[i].db, index, true);
                }
#endif
            }
            if (loggerTypeFile == lg->loggers[i].type)
            {
#if defined(_PALM_OS)
                assert(lg->loggers[i].file);
                HostFPutS(txt, lg->loggers[i].file);
                HostFFlush(lg->loggers[i].file);
#endif
            }

            if (loggerTypeDebugger == lg->loggers[i].type)
            {
#if defined(_PALM_OS)
                DbgMessage(txt);
#elif defined(_WIN32_WCE)
                OutputDebugString(txt);
#else
# error "Define version of DebuggerLogSink appropriate for your system."
#endif            
            }
        }
    }
}

void Log(LogLevel level, const char_t *txt, bool fFinishLine)
{
    LoggingGlobals *lg = GetLoggingGlobals(true);
    if (NULL == lg)
    {
        // no loggers created - ignore request
        return;
    }
    if (!FNeedsLogging(lg, level))
        return;

    DynStr *logStr = &(lg->curLine);
    bool fFirst = false;
    if (0 == DynStrLen(logStr))
        fFirst = true;
    AddToLog(logStr, txt, fFirst);

    if (fFinishLine)
    {
        DynStrAppendChar(logStr, _T('\n'));
        LogRaw(lg, level, DynStrGetCStr(logStr));
        DynStrTruncate(logStr, 0);
    }
}

void LogUlong(LogLevel level, unsigned long num, bool fFinishLine)
{
    char_t buffer[26] = {0};
    tprintf(buffer, _T("%lu (0x%08lx)"), num, num);
    Log(level, buffer, fFinishLine);
}

void LogStrUlong(LogLevel level, const char_t *txt, unsigned long num)
{
    Log(level, txt, false);
    LogUlong(level, num, true);
}

#if defined(_PALM_OS)

void ArsLexis::cleanAllocationLogging()
{
    HostFILEType * file = NULL;
    UInt32 creatorId = Application::creator();
    Err error = FtrGet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32*>(&file));
    if (errNone == error && NULL != file)
        HostFClose(file);
    FtrUnregister(creatorId, Application::featureAllocationLoggerPointer);
}

#if !defined(ALLOCATION_LOG_PATH)
# error "Define ALLOCATION_LOG_PATH in yer Target.h"
#endif

void ArsLexis::logAllocation(void* ptr, size_t size, bool free, const char* fileName, int line)
{
#if defined(ARSLEXIS_DEBUG_MEMORY_ALLOCATION)
    HostFILEType *  file = NULL;
    UInt32          creatorId = Application::creator();

    Err  error = FtrGet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32*>(&file));
    if (errNone != error)
    {
        file = HostFOpen(ALLOCATION_LOG_PATH, "w");
        if (file)
            FtrSet(creatorId, Application::featureAllocationLoggerPointer, reinterpret_cast<UInt32>(file));
    }
    if (NULL == file)
        return;

    if (free)
    {
        // sometimes free is called with NULL, so don't log those
        if ( NULL != ptr)
            HostFPrintF(file, "-\t0x%08lx\n", ptr);
    }
    else
    {
        if (NULL == fileName)
            fileName = "(Unknown)";
        HostFPrintF(file, "+\t0x%08lx\t%ld\t%s: %hd\n", ptr, size, fileName, line);
    }
    HostFFlush(file);
#endif    
}

#else // _PALM_OS
// TODO: implement Win CE version
void ArsLexis::logAllocation(void* ptr, size_t size, bool free, const char* fileName, int line)
{
}
#endif // _PALMOS
