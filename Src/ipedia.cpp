
#include "iPediaApplication.hpp"

void std::__msl_error(const char* str)
{
    ErrFatalDisplay(str);
    ErrThrow(sysErrParamErr);
}

void ArsLexis::handleBadAlloc()
{
    ErrThrow(memErrNotEnoughSpace);    
}

void ArsLexis::logAllocation(void* ptr, bool free, const char* file, int line)
{
/*
    iPediaApplication* app=static_cast<iPediaApplication*>(Application::getInstance(iPediaApplication::creatorId));
    if (app && app->logAllocation_)
    {
        app->logAllocation_=false;
        char buff[12];
        StrPrintF(buff, "0x%08lx", ptr);
        if (!free)
            app->log().info()<<"+\t"<<buff<<"\t"<<file<<": "<<line;
        else
            app->log().info()<<"-\t"<<buff;
        app->logAllocation_=true;
    }
*/    
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication, romIncompatibleAlert>(cmd, cmdPBP, launchFlags);
}
