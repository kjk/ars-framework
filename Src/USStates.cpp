#include <USStates.hpp>

namespace ArsLexis{
    
    ArsLexis::char_t* states[] = { 
                                    "AL", "Alabama", 
                                    "AK", "Alaska", 
                                    "AZ", "Arizona", 
                                    "AR", "Arkansas", 
                                    "CA", "California", 
                                    "CO", "Colorado", 
                                    "CT", "Connecticut", 
                                    "DE", "Delaware", 
                                    "DC", "District of Col.", 
                                    "FL", "Florida", 
                                    "GA", "Georgia", 
                                    "HI", "Hawaii", 
                                    "ID", "Idaho", 
                                    "IL", "Illinois", 
                                    "IN", "Indiana", 
                                    "IA", "Iowa", 
                                    "KS", "Kansas", 
                                    "KY", "Kentucky", 
                                    "LA", "Louisiana", 
                                    "ME", "Maine", 
                                    "MD", "Maryland", 
                                    "MA", "Massachusetts", 
                                    "MI", "Michigan", 
                                    "MN", "Minnesota", 
                                    "MS", "Mississippi", 
                                    "MO", "Missouri", 
                                    "MT", "Montana", 
                                    "NE", "Nebraska", 
                                    "NV", "Nevada",
                                    "NH", "New Hampshire", 
                                    "NJ", "New Jersey", 
                                    "NM", "New Mexico", 
                                    "NY", "New York", 
                                    "NC", "North Carolina", 
                                    "ND", "North Dakota", 
                                    "OH", "Ohio", 
                                    "OK", "Oklahoma", 
                                    "OR", "Oregon", 
                                    "PA", "Pennsylvania", 
                                    "RI", "Rhode Island", 
                                    "SC", "South Carolina", 
                                    "SD", "South Dakota", 
                                    "TN", "Tennessee", 
                                    "TX", "Texas", 
                                    "UT", "Utah", 
                                    "VT", "Vermont", 
                                    "VA", "Virginia", 
                                    "WA", "Washington", 
                                    "WV", "West Virginia", 
                                    "WI", "Wisconsin", 
                                    "WY", "Wyoming", 
                                    "", "--Provinces--", 
                                    "AB", "Alberta", 
                                    "BC", "British Columbia", 
                                    "MB", "Manitoba", 
                                    "NB", "New Brunswick", 
                                    "NL", "Newfoundland & Labrador", 
                                    "NT", "Northwest Territories", 
                                    "NS", "Nova Scotia", 
                                    "NU", "Nunavut", 
                                    "ON", "Ontario", 
                                    "PE", "Prince Edward Island", 
                                    "QC", "Quebec", 
                                    "SK", "Saskatchewan", 
                                    "YT", "Yukon"
        };

    int getStatesCount()
    {
        return sizeof(states)/sizeof(states[0])/2;
    }

    void getStateName(int pos, ArsLexis::String& nameOut)
    {   
        assert(pos >= 0 && pos < getStatesCount());
        nameOut.assign(states[pos*2+1]);
    }

    void getStateSymbol(int pos, ArsLexis::String& symbolOut) 
    {
        assert(pos >= 0 && pos < getStatesCount());
        symbolOut.assign(states[pos*2]);
    }
}