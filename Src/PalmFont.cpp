#include "PalmFont.hpp"

namespace ArsLexis
{

    PalmFont::PalmFontImpl::~PalmFontImpl()
    {
        assert(0==refCount_);
    }

    PalmFont::PalmFont(FontID fontId):
        impl_(stdFont==fontId?0:new PalmFontImpl(fontId))
    {
        if (impl_)
            impl_->attach();
    }
    
    PalmFont::PalmFont():
        impl_(0)
    {
    }
    
    PalmFont::PalmFontImpl* PalmFont::PalmFontImpl::clone() const
    {
        PalmFontImpl* result=new PalmFontImpl(fontId_);
        result->effects_=effects_;
        return result;
    }
    
    PalmFont& PalmFont::operator=(const PalmFont& ref)
    {
        if (ref.impl_!=impl_)
        {
            if (impl_)
                impl_->release();
            impl_=ref.impl_;
            if (impl_)
                impl_->attach();
        }
        return *this;
    }    
    
    
    FontID PalmFont::withEffects() const
    {
        FontID normal=fontId();
        FontID result=normal;
        if (impl_ && ((impl_->effects().weight()!=FontEffects::weightPlain) || impl_->effects().italic()))
        {
            switch (normal)
            {
                case stdFont:
                    result=boldFont;
                    break;
                case largeFont:
                    result=largeBoldFont;
                    break;
            }
        }
        return result;
    }
    
    void PalmFont::selfish()
    {
        if (0==impl_)
        {
            impl_=new PalmFontImpl(stdFont);
            impl_->attach();
        }
        else if (impl_->refCount()>1)
        {
            PalmFontImpl* newImpl=impl_->clone();
            impl_->release();
            impl_=newImpl;
            impl_->attach();
        }
    }

    PalmFont PalmFont::getSymbolFont()
    {
        return PalmFont(symbolFont);
    }
   
    
}    