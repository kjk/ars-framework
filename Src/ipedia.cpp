
#include "iPediaApplication.hpp"

void std::__msl_error(const char* str)
{
    ErrFatalDisplay(str);
    ErrThrow(sysErrParamErr);
}

void ArsLexis::handleBadAlloc()
{
    FrmAlert(notEnoughMemoryAlert);
    ErrThrow(memErrNotEnoughSpace);    
}

void ArsLexis::logAllocation(void* ptr, bool free, const char* file, int line)
{
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication, appFileCreator>(cmd, cmdPBP, launchFlags);
}
