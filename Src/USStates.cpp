#include <USStates.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
#endif

enum {
    stateAbbrevLength=3,
    stateNameLength=24,
};

typedef const char_t StateAbbrevField_t[stateAbbrevLength];
typedef const char_t StateNameField_t[stateNameLength];

struct StateArrayEntry {
    const StateAbbrevField_t abbrev;
    const StateNameField_t name; 
};
    
static const StateArrayEntry states[]={
    {"AL", "Alabama"}, 
    {"AK", "Alaska"}, 
    {"AB", "Alberta"}, 
    {"AZ", "Arizona"}, 
    {"AR", "Arkansas"}, 
    {"BC", "British Columbia"}, 
    {"CA", "California"}, 
    {"CO", "Colorado"}, 
    {"CT", "Connecticut"}, 
    {"DE", "Delaware"}, 
    {"DC", "District of Col."}, 
    {"FL", "Florida"}, 
    {"GA", "Georgia"}, 
    {"HI", "Hawaii"}, 
    {"ID", "Idaho"}, 
    {"IL", "Illinois"}, 
    {"IN", "Indiana"}, 
    {"IA", "Iowa"}, 
    {"KS", "Kansas"}, 
    {"KY", "Kentucky"}, 
    {"LA", "Louisiana"}, 
    {"ME", "Maine"}, 
    {"MB", "Manitoba"}, 
    {"MD", "Maryland"}, 
    {"MA", "Massachusetts"}, 
    {"MI", "Michigan"}, 
    {"MN", "Minnesota"}, 
    {"MS", "Mississippi"}, 
    {"MO", "Missouri"}, 
    {"MT", "Montana"}, 
    {"NE", "Nebraska"}, 
    {"NV", "Nevada"},
    {"NB", "New Brunswick"}, 
    {"NH", "New Hampshire"}, 
    {"NJ", "New Jersey"}, 
    {"NM", "New Mexico"}, 
    {"NY", "New York"}, 
    {"NL", "Newfoundland & Labrador"}, 
    {"NC", "North Carolina"}, 
    {"ND", "North Dakota"}, 
    {"NT", "Northwest Territories"}, 
    {"NS", "Nova Scotia"}, 
    {"NU", "Nunavut"}, 
    {"OH", "Ohio"}, 
    {"OK", "Oklahoma"}, 
    {"ON", "Ontario"}, 
    {"OR", "Oregon"}, 
    {"PA", "Pennsylvania"}, 
    {"PE", "Prince Edward Island"}, 
    {"QC", "Quebec"}, 
    {"RI", "Rhode Island"}, 
    {"SK", "Saskatchewan"}, 
    {"SC", "South Carolina"}, 
    {"SD", "South Dakota"}, 
    {"TN", "Tennessee"}, 
    {"TX", "Texas"}, 
    {"UT", "Utah"}, 
    {"VT", "Vermont"}, 
    {"VA", "Virginia"}, 
    {"WA", "Washington"}, 
    {"WV", "West Virginia"}, 
    {"WI", "Wisconsin"}, 
    {"WY", "Wyoming"}, 
    {"YT", "Yukon"}
};

#define STATES_COUNT sizeof(states)/sizeof(states[0])

int getStatesCount()
{
    return STATES_COUNT;
}

char_t *getStateName(int pos)
{
    assert(pos >= 0 && pos < STATES_COUNT);
    return (char_t*)states[pos].name;
}

char_t *getStateSymbol(int pos)
{
    assert(pos >= 0 && pos < STATES_COUNT);
    return (char_t*)states[pos].abbrev;
}

// always return index value in (0, statesCount-1)
int getIndexByFirstChar(ArsLexis::char_t inChar)
{
    inChar = ArsLexis::toLower(inChar);
    for (int i = 0; i < STATES_COUNT; i++)
    {
        char_t foundChar = ArsLexis::toLower(states[i].name[0]);
        if (inChar <= foundChar)
            return i;
    }
    return STATES_COUNT-1;
}
