#ifndef ARSLEXIS_HYPERLINK_HANDLER_BASE_HPP__
#define ARSLEXIS_HYPERLINK_HANDLER_BASE_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

struct Point;

class HyperlinkHandlerBase
{
public:

    virtual void handleHyperlink(const char* link, ulong_t len, const Point* point);

    virtual ~HyperlinkHandlerBase();
    
};

#endif