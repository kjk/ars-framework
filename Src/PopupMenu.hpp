#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <FormObject.hpp>
#include <Definition.hpp>

class PopupMenuItemDrawHandler: public ArsLexis::List::CustomDrawHandler
{
public:

    struct Item
    {
        ArsLexis::char_t* text;
        ArsLexis::char_t* hyperlink;
        bool active;
        bool bold;
        
        Item();
        
        ~Item();
    };        
       
    Item* items;
    UInt16 count;
    RGBColorType inactiveTextColor;
    
    PopupMenuItemDrawHandler();
    
    ~PopupMenuItemDrawHandler();
    
    UInt16 maxTextWidth() const;
    
    void drawItem(ArsLexis::Graphics& graphics, ArsLexis::List& list, uint_t item, const ArsLexis::Rectangle& itemBounds);
    
    uint_t itemsCount() const;
    
    bool itemsFromString(const ArsLexis::char_t* data, long length);
    
};

class PopupMenu
{
    UInt16 prevFocusIndex_;

public:

    Definition::HyperlinkHandler* hyperlinkHandler;
    PopupMenuItemDrawHandler itemDrawHandler;
    ArsLexis::List list;

    PopupMenu(ArsLexis::Form& form);
    
    ~PopupMenu();
    
    bool handleEventInForm(EventType& event);

    Int16 popup(UInt16 id, const ArsLexis::Point& point);
    
};



#endif