
#include "iPediaApplication.hpp"
#include "ipedia_Rsc.h"

void std::__msl_error(const char* str)
{
	ErrFatalDisplay(str);
	abort();
}

void ArsLexis::handleBadAlloc()
{
    FrmAlert(notEnoughMemoryAlert);
    std::abort();
}

void ArsLexis::logAllocation(void*, bool )
{
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication, appFileCreator>(cmd, cmdPBP, launchFlags);
}
