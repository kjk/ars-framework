#ifndef __LOOKUP_HISTORY_HPP__
#define __LOOKUP_HISTORY_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <list>

namespace ArsLexis
{
    class PrefsStoreReader;
    class PrefsStoreWriter;
}    

class LookupHistory
{

    // Not quite effective. But using std::deque for this purpose would increase code size significantly.
    typedef std::list<ArsLexis::String> TermHistory_t;
    TermHistory_t termHistory_;
    TermHistory_t::iterator historyPosition_;
    
public:

    enum {
        maxLength=25, 
        // Ugly.
        reservedPrefIdCount=maxLength+2
    };

    LookupHistory();
    
    ~LookupHistory();
    
    bool hasPrevious() const
    {return historyPosition_!=termHistory_.begin();}
    
    bool hasNext() const;
    
    void replaceAllNext(const ArsLexis::String& term);
    
    void moveNext(const ArsLexis::String& term)
    {
        if (hasNext()) 
            *(++historyPosition_)=term;
    }
    
    void movePrevious(const ArsLexis::String& term)
    {
        if (hasPrevious())
            *(--historyPosition_)=term;
    }
    
    const ArsLexis::String& currentTerm() const
    {
        assert(!termHistory_.empty());
        return *historyPosition_;
    }
    
    const ArsLexis::String& previousTerm() const
    {
        assert(hasPrevious());
        TermHistory_t::const_iterator it=historyPosition_;
        return *(--it);
    }
    
    const ArsLexis::String& nextTerm() const
    {
        assert(hasNext());
        TermHistory_t::const_iterator it=historyPosition_;
        return *(++it);
    }
    
    void clearPast()
    {termHistory_.erase(termHistory_.begin(), historyPosition_);}
    
    bool hasCurrentTerm() const
    {return historyPosition_!=termHistory_.end();}
    
    Err serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const;
    
    Err serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId);
    
};

#endif