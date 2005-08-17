#include <HyperlinkHandlerBase.hpp>

#if defined(_PALM_OS)
#pragma segment Segment1
#endif

HyperlinkHandlerBase::~HyperlinkHandlerBase()
{}

void HyperlinkHandlerBase::handleHyperlink(const char* link, ulong_t len, const Point*)
{
}

