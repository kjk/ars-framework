
#include "iPediaApplication.hpp"
#include "ipedia_Rsc.h"

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return ArsLexis::Application::main<iPediaApplication>(cmd, cmdPBP, launchFlags);
}
