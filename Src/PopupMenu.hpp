#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <Debug.hpp>

#ifdef _PALM_OS
#include <FormObject.hpp>
#endif

#include <Definition.hpp>

class PopupMenuModel
#ifdef _PALM_OS
: public List::CustomDrawHandler
#endif
{
public:

    struct Item
    {
        char_t* text;
        char* hyperlink;
        bool active;
        bool bold;
        bool separator;
        bool underlined;
                
        Item();
        
        ~Item();
    };        
       
    Item* items;
    ulong_t count;

#ifdef _PALM_OS    
    RGBColorType inactiveTextColor;
#endif
    
    PopupMenuModel();
    
    ~PopupMenuModel();
    
    uint_t maxTextWidth() const;

#ifdef _PALM_OS    
    void drawItem(Graphics& graphics, List& list, uint_t item, const ArsRectangle& itemBounds);
#endif
    
    ulong_t itemsCount() const;
    
    // Passes ownership of items to PopupMenuModel.
    void setItems(Item* items, ulong_t itemsCount);

    bool itemsFromString(const char* data, ulong_t length);
    
};

#ifdef _PALM_OS
class PopupMenu
{
    UInt16 prevFocusIndex_;
    PopupMenuModel* model_;

public:

    HyperlinkHandlerBase* hyperlinkHandler;
    List list;
    Int16 initialSelection;

    PopupMenu(Form& form);
    
    ~PopupMenu();
    
    bool handleEventInForm(EventType& event);

    Int16 popup(UInt16 id, const Point& point);
    
    enum ModelOwnerFlag
    {
        ownModelNot,
        ownModel
    };
       
    
    // Passes model ownership to PopupMenu and deletes previous model
    void setModel(PopupMenuModel* model, ModelOwnerFlag owner = ownModel);
    
    PopupMenuModel* model() {return model_;}

private:
    
    ModelOwnerFlag modelOwner_;    
   
};
#endif

enum PopupMenuItemFlag 
{
    popupMenuItemInactive = 1,
    popupMenuItemBold = 2,
    popupMenuItemSeparator = 4,
    popupMenuItemUnderlined = 8
};

status_t PopupMenuHyperlinkCreate(char*& hyperlink, ulong_t& length, const char* prefix, ulong_t itemsCount);

status_t PopupMenuHyperlinkAppendItem(char*& hyperlink, ulong_t& length, const char_t* text, const char* link, ulong_t itemFlags);

#ifdef _WIN32
long PopupMenuShow(const PopupMenuModel& model, const Point& point, HWND wnd);

#ifndef NDEBUG
void test_PopupMenu(HWND wnd);
#endif

#endif

#endif