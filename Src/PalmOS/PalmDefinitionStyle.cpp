#include <DefinitionStyle.hpp>
#include <Graphics.hpp>
#include <Debug.hpp>
#include <Text.hpp>

struct StaticStyleEntry
{
    const char* name;
    const DefinitionStyle style;
    
    // For use by binary search (std::lower_bound())
    bool operator<(const StaticStyleEntry& other) const
    {
        return tstrcmp(name, other.name) < 0;
    }
    
};

#define COLORNOTDEFINED  {-1,0,0,0}
#define RGB(r,g,b)  {0,r,g,b}
#define WHITE    RGB(255,255,255)
#define BLACK    RGB(0,0,0)
#define GREEN    RGB(0,196,0)
#define BLUE     RGB(0,0,255)
#define RED      RGB(255,0,0)
#define YELLOW   RGB(255,255,0)
#define GRAY     RGB(127,127,127)

#define FONTNOTDEFINED  FontID(-1)
#define NOTDEF          DefinitionStyle::notDefined
#define TRUE            DefinitionStyle::yes
#define FALSE           DefinitionStyle::no
#define UNDERLINENOTDEF UnderlineModeTag(-1)

#define COLOR(name, color) \
    {(name), {color, COLORNOTDEFINED, FONTNOTDEFINED, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, UNDERLINENOTDEF}}
#define COLORBOLD(name, color) \
    {(name), {color, COLORNOTDEFINED, FONTNOTDEFINED, TRUE, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, UNDERLINENOTDEF}}
#define COLORANDFONT(name, color, font) \
    {(name), {color, COLORNOTDEFINED, font, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, UNDERLINENOTDEF}}
#define COLORANDFONTBOLD(name, color, font) \
    {(name), {color, COLORNOTDEFINED, font, TRUE, NOTDEF, NOTDEF, NOTDEF, NOTDEF, NOTDEF, UNDERLINENOTDEF}}

static const StaticStyleEntry staticStyleTable[] =
{
    //TODO: add more and more:)
    COLOR("black",BLACK),
    COLOR("blue",BLUE),
    COLORBOLD("bold",COLORNOTDEFINED),
    COLOR("gray",GRAY),
    COLOR("green",GREEN),
    {"hyperlink", {BLUE, COLORNOTDEFINED, FONTNOTDEFINED, NOTDEF, NOTDEF, NOTDEF, TRUE, NOTDEF, NOTDEF, grayUnderline}},
    COLORANDFONT("large", COLORNOTDEFINED, largeFont),
    COLOR("red",RED)
};



uint_t getStaticStyleCount()
{
    return ARRAY_SIZE(staticStyleTable);
}

const char* getStaticStyleName(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleTable));
    return staticStyleTable[index].name;
}

const DefinitionStyle* getStaticStyle(uint_t index)
{
    assert(index < ARRAY_SIZE(staticStyleTable));
    return &staticStyleTable[index].style;
}

const DefinitionStyle* getStaticStyle(const char* name, uint_t length)
{
    if (NULL == name || 0 == length)
        return NULL;
    char* nameBuf = StringCopy2N(name, length);
    if (NULL == nameBuf)
        return NULL;
    
    StaticStyleEntry entry = COLOR(nameBuf, COLORNOTDEFINED);
    const StaticStyleEntry* end = staticStyleTable + ARRAY_SIZE(staticStyleTable);
    const StaticStyleEntry* res = std::lower_bound(staticStyleTable, end, entry);

    free(nameBuf);

    if (end == res)
        return NULL;
    return &res->style; 
}

DefinitionStyle* parseStyle(const char* style, ulong_t length)
{
    //TODO: 



    return new DefinitionStyle();
}

#ifdef DEBUG
void test_StaticStyleTable()
{
    // Validate that fields_ are sorted.
    for (uint_t i = 0; i < ARRAY_SIZE(staticStyleTable); ++i)
    {
        if (i > 0 && staticStyleTable[i] < staticStyleTable[i-1])
        {
            const char_t* prevName = staticStyleTable[i-1].name;
            const char_t* nextName = staticStyleTable[i].name;
            assert(false);
        }
    }
}
#endif
