#ifndef __IPEDIA_HYPERLINK_HANDLER_HPP__
#define __IPEDIA_HYPERLINK_HANDLER_HPP__

#include "Definition.hpp"

class iPediaHyperlinkHandler: public Definition::HyperlinkHandler
{
    
    UInt16 viewerCardNo_;
    LocalID viewerDatabaseId_;
    
    bool handleExternalHyperlink(const ArsLexis::String& url);
    bool handleTermHyperlink(const ArsLexis::String& term);
    
public:

    iPediaHyperlinkHandler(UInt16 viewerCardNo=0, LocalID viewerDatabaseId=0);
    
    void setViewerLocation(UInt16 viewerCardNo, LocalID viewerDatabaseId)
    {viewerCardNo_=viewerCardNo; viewerDatabaseId_=viewerDatabaseId;}

    void handleHyperlink(Definition& definition, DefinitionElement& hyperlinkElement);
    
};

#endif