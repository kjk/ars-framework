#ifndef __ARSLEXIS_PALM_FONT_HPP__
#define __ARSLEXIS_PALM_FONT_HPP__

#include <PalmOS.h>
#include <BaseTypes.hpp>
#include <FontEffects.hpp>
#include <Utility.hpp>

namespace ArsLexis
{

    class PalmFont
    {
    
        class PalmFontImpl: private NonCopyable
        {
            mutable uint_t refCount_;
            FontID fontId_;
            FontEffects effects_;
            
        public:
        
            ~PalmFontImpl();
            
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
    
        PalmFont();
    
        PalmFont(FontID fontId);
        
        PalmFont(const PalmFont& ref):
            impl_(ref.impl_)
        {
            if (impl_)
                impl_->attach();
        }
        
        ~PalmFont()
        {
            if (impl_)
                impl_->release();
        }
        
        PalmFont& operator=(const PalmFont& ref);
        
        FontID fontId() const
        {return impl_?impl_->fontId():stdFont;}
        
        FontEffects effects() const
        {return impl_?impl_->effects():FontEffects();}       
        
        FontID withEffects() const;
        
        void addEffects(const FontEffects& effects)
        {
            if (!effects.empty())
            {
                selfish();
                impl_->addEffects(effects);
            }                
        }

        void setEffects(const FontEffects& effects)
        {
            if (impl_ || !effects.empty())
            {
                selfish();
                impl_->effects()=effects;
            }                
        }
        
        void setFontId(FontID fontId)
        {
            if (impl_ || stdFont!=fontId)
            {
                selfish();
                impl_->setFontId(fontId);
            }                
        }
        
        static PalmFont getSymbolFont();

    };

}

#endif