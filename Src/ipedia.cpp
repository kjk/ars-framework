
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
    Application* app=Application::getInstance(appFileCreator);
    if (app)
        app->abort();
    std::abort();
}

void ArsLexis::logAllocation(void*, bool )
{
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication, appFileCreator>(cmd, cmdPBP, launchFlags);
}
