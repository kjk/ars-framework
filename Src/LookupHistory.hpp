#ifndef __LOOKUP_HISTORY_HPP__
#define __LOOKUP_HISTORY_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <list>

class LookupHistory
{
    enum {historyLength_=20};
    typedef std::list<ArsLexis::String> TermHistory_t;
    TermHistory_t termHistory_;
    TermHistory_t::iterator historyPosition_;
    
public:

    LookupHistory();
    
    ~LookupHistory();
    
    bool hasPrevious() const
    {return historyPosition_!=termHistory_.begin();}
    
    bool hasNext() const
    {
        TermHistory_t::const_iterator last=termHistory_.end();
        if (!termHistory_.empty())
            --last;
        return historyPosition_!=last;
    }
    
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
    
    bool empty() const
    {return termHistory_.empty();}
    
};

#endif