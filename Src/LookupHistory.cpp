#include "LookupHistory.hpp"

using ArsLexis::String;

LookupHistory::LookupHistory():
    historyPosition_(termHistory_.end())
{
}

LookupHistory::~LookupHistory()
{
}
    
void LookupHistory::replaceAllNext(const ArsLexis::String& term)
{
    if (!termHistory_.empty())
        termHistory_.erase(++historyPosition_, termHistory_.end());
    termHistory_.push_back(term);
    historyPosition_=termHistory_.end();
    --historyPosition_;
    while (termHistory_.size()>historyLength_)
        termHistory_.pop_front();
}
