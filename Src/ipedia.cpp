
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

void ArsLexis::logAllocation(void*, bool, const char*, int)
{
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication, romIncompatibleAlert>(cmd, cmdPBP, launchFlags);
}
