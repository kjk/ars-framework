#include <FormObject.hpp>

namespace ArsLexis
{

    FormObject::FormObject(Form& form, UInt16 id):
        form_(&form),
        id_(frmInvalidObjectId),
        index_(frmInvalidObjectId),
        object_(0)
    {
        if (id!=frmInvalidObjectId)
            attach(id);
    }
    
    FormObject::~FormObject()
    {}
    
    void FormObject::attach(UInt16 id)
    {
        assert(frmInvalidObjectId!=id);
        index_=FrmGetObjectIndex(*form_, id_=id);
        assert(frmInvalidObjectId!=index_);
        object_=FrmGetObjectPtr(*form_, index_);
        assert(0!=object_);
    }
    
    void FormObject::bounds(Rectangle& out) const
    {
        RectangleType rect;
        FrmGetObjectBounds(*form_, index(), &rect);
        out=rect;
    }
    
    void FormObject::setBounds(const Rectangle& bounds)
    {
        assert(valid());
        RectangleType rect=toNative(bounds);
        FrmSetObjectBounds(*form_, index(), &rect);
    }

    void FormObject::attachByIndex(UInt16 index)
    {
        assert(frmInvalidObjectId!=index);
        id_=FrmGetObjectId(*form_, index_=index);
        assert(frmInvalidObjectId!=id_);
        object_=FrmGetObjectPtr(*form_, index_);
        assert(0!=object_);
    }
    
#pragma mark -
    
    void Field::replace(const char* text, uint_t length)
    {
        erase();
        insert(text, std::min(maxLength(), length));
    }

#pragma mark -
#pragma mark List

    // At any given time an element of the list is selected. This function
    // will set a new item selected of the pos curSelected + delta. delta
    // can be negative. If asked to set selection to a inexistent item (<0 or
    // >number of items) sets selection to the first/last item (this is so that
    // the caller doesn't have to worry about this stuff)
    void List::setSelectionDelta(int delta)
    {
        assert(0!=delta);

        uint_t curSelection;

        if (noListSelection==getSelection())
            curSelection = 0;
        else
            curSelection = (uint_t)getSelection();

        if (delta>0)
        {
            curSelection += (uint_t)delta;
            if (curSelection>numberOfItems()-1)
                curSelection=numberOfItems()-1;
        }
        else
        {
            delta = -delta;
            if (curSelection<(uint_t)delta)
                curSelection=0;
            else
                curSelection-=delta;
        }
        setSelection(curSelection);
        makeItemVisible(curSelection);
    }


}
