#include "iPediaHyperlinkHandler.hpp"
#include "iPediaApplication.hpp"
#include "MainForm.hpp"

using namespace ArsLexis;

void iPediaHyperlinkHandler::handleExternalHyperlink(const ArsLexis::String& url)
{
    if (viewerDatabaseId_)
    {
        UInt16 urlLength=url.length();
        char* urlCopy=static_cast<char*>(MemPtrNew(urlLength+1));
        if (urlCopy)
        {
            MemMove(urlCopy, url.data(), urlLength);
            urlCopy[urlLength]=chrNull;
            Err error=MemPtrSetOwner(urlCopy, 0);
            assert(!error);
            error=SysUIAppSwitch(viewerCardNo_, viewerDatabaseId_, sysAppLaunchCmdGoToURL, urlCopy);
            if (error)
            {
                //! @todo Handle error running www viewer.
            }
        }
    }
}

void iPediaHyperlinkHandler::handleTermHyperlink(const ArsLexis::String& term)
{
    iPediaApplication& app=static_cast<iPediaApplication&>(Application::instance());
    MainForm* form=static_cast<MainForm*>(app.getOpenForm(mainForm));
    if (form)
        form->lookupTerm(term);
}


iPediaHyperlinkHandler::iPediaHyperlinkHandler(UInt16 viewerCardNo, LocalID viewerDatabaseId):
    viewerCardNo_(viewerCardNo),
    viewerDatabaseId_(viewerDatabaseId)
{
}

void iPediaHyperlinkHandler::handleHyperlink(const ArsLexis::String& resource, HyperlinkType type)
{
    assert(hyperlinkBookmark!=type);
    if (hyperlinkExternal==type)
        handleExternalHyperlink(resource);
    else
        handleTermHyperlink(resource);
}
