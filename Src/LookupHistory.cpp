#include "LookupHistory.hpp"
#include <PrefsStore.hpp>

using ArsLexis::String;

LookupHistory::LookupHistory():
    historyPosition_(termHistory_.end())
{
}

void LookupHistory::replaceAllNext(const ArsLexis::String& term)
{
    if (historyPosition_!=termHistory_.end() && !termHistory_.empty())
        termHistory_.erase(++historyPosition_, termHistory_.end());
    termHistory_.push_back(term);
    historyPosition_=termHistory_.end();
    --historyPosition_;
    while (termHistory_.size()>maxLength)
        termHistory_.pop_front();
}

Err LookupHistory::serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const
{
    UInt16 itemCount=termHistory_.size(); 
    Err error;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, itemCount)))
        goto OnError;
    UInt16 currentItem=0;
    TermHistory_t::const_iterator end(termHistory_.end());
    for (TermHistory_t::const_iterator it(termHistory_.begin()); it!=end; ++it)
    {
        if (historyPosition_!=it)
            ++currentItem;
        if (errNone!=(error=writer.ErrSetStr(uniqueId++, it->c_str())))
            goto OnError;
    }
    if (itemCount>=0 && currentItem==itemCount)
        currentItem--;
    if (errNone!=(error=writer.ErrSetUInt16(uniqueId++, currentItem)))
        goto OnError;
OnError:    
    return error;
}

Err LookupHistory::serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId)
{
    LookupHistory tmp;
    Err error;
    UInt16 itemCount;
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &itemCount)))
        goto OnError;
    if (maxLength<itemCount)
    {
        error=psErrDatabaseCorrupted;
        goto OnError;
    }
    for (UInt16 i=0; i<itemCount; ++i)
    {
        const char* p=0;
        if (errNone!=(error=reader.ErrGetStr(uniqueId++, &p)))
            goto OnError;
        tmp.termHistory_.push_back(p);
    }
    UInt16 lastItem;
    if (errNone!=(error=reader.ErrGetUInt16(uniqueId++, &lastItem)))
        goto OnError;
    if (itemCount>0 && lastItem>=itemCount)
    {
        error=psErrDatabaseCorrupted;
        goto OnError;
    }
    if (0!=itemCount)
    {
        tmp.historyPosition_=tmp.termHistory_.begin();
        do 
        {
            ++tmp.historyPosition_;
        } while (--itemCount);
        tmp.termHistory_.erase(tmp.historyPosition_, tmp.termHistory_.end());           
    }        
    std::swap(termHistory_, tmp.termHistory_);
    historyPosition_=termHistory_.end();
    
OnError:    
    return error;
}

bool LookupHistory::hasNext() const
{
    TermHistory_t::const_iterator last=termHistory_.end();
    if (historyPosition_==last)
        return false;
    if (!termHistory_.empty())
        --last;
    return historyPosition_!=last;
}
    
