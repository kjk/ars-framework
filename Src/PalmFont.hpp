#ifndef __ARSLEXIS_PALM_FONT_HPP__
#define __ARSLEXIS_PALM_FONT_HPP__

#include <PalmOS.h>
#include "BaseTypes.hpp"
#include "FontEffects.hpp"

namespace ArsLexis
{

    class PalmFont
    {
        
        class PalmFontImpl
        {
            mutable uint_t refCount_;
            FontID fontId_;
            FontEffects effects_;
            
        protected:
            
            ~PalmFontImpl();
            
        public:
        
            uint_t refCount() const
            {return refCount_;}
        
            PalmFontImpl(FontID fontId):
                refCount_(0),
                fontId_(fontId)
            {}
            
            void attach() const
            {++refCount_;}
            
            void release()
            {
                if (! (--refCount_))
                    delete this;
            }
           
            void setFontId(FontID fontId)
            {fontId_=fontId;}
            
            FontID fontId() const
            {return fontId_;}
            
            const FontEffects& effects() const
            {return effects_;}
            
            FontEffects& effects() 
            {return effects_;}
            
            PalmFontImpl* clone() const;
            
            void addEffects(const FontEffects& effects)
            {effects_+=effects;}
            
            
        };
        
        PalmFontImpl* impl_;
        
        void selfish();
    
    public:
    
        PalmFont(FontID fontId=stdFont);
        
        PalmFont(const PalmFont& ref):
            impl_(ref.impl_)
        {impl_->attach();}
        
        ~PalmFont()
        {impl_->release();}
        
        PalmFont& operator=(const PalmFont& ref);
        
        FontID fontId() const
        {return impl_->fontId();}
        
        const FontEffects& effects() const
        {return impl_->effects();}       
        
        FontID withEffects() const;
        
        void addEffects(const FontEffects& effects)
        {
            selfish();
            impl_->addEffects(effects);
        }

        void setEffects(const FontEffects& effects)
        {
            selfish();
            impl_->effects()=effects;
        }
        
        void setFontId(FontID fontId)
        {
            selfish();
            impl_->setFontId(fontId);
        }
        
        friend class Graphics;
    };

}

#endif