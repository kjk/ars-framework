#include "PalmFont.hpp"

namespace ArsLexis
{
    
    PalmFont::PalmFontImpl::~PalmFontImpl()
    {
        assert(0==refCount_);
    }

    PalmFont::PalmFont(FontID fontId):
        impl_(new PalmFontImpl(fontId))
    {
        impl_->attach();
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
            impl_->release();
            impl_=ref.impl_;
            impl_->attach();
        }
        return *this;
    }    
    
    
    FontID PalmFont::withEffects() const
    {
        FontID normal=fontId();
        FontID result=normal;
        if ((effects().weight()!=FontEffects::weightPlain) || effects().italic())
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
        if (impl_->refCount()>1)
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