#ifndef ARSLEXIS_TEXT_RENDERER_HPP__
#define  ARSLEXIS_TEXT_RENDERER_HPP__

#include <FormGadget.hpp>
#include <Definition.hpp>

namespace ArsLexis 
{
    class TextRenderer: public FormGadget 
    {
        Definition definition_;
        RenderingPreferences* renderingPreferences_;
        
    public:
 
         explicit TextRenderer(Form& form, UInt16 id = frmInvalidObjectId);

        ~TextRenderer();

        typedef Definition::const_iterator const_iterator;
        typedef Definition::iterator iterator;

        iterator  begin() {return definition_.begin();}
        const_iterator begin() const {return definition_.begin();}
        iterator end() {return definition_.end();}
        const_iterator end() const {return definition_.end();}
        bool empty() const {return definition_.empty();}

        typedef Definition::Elements_t Elements_t;
        void replaceElements(Elements_t& elements) {definition_.replaceElements(elements);}

        uint_t linesCount() const {return definition_.totalLinesCount();}

        uint_t visibleLinesCount() const {return definition_.shownLinesCount();}

        uint_t topLine() const {return definition_.firstShownLine();}
	
        void clear() {definition_.clear();}

        typedef Definition::HyperlinkHandler HyperlinkHandler;
        HyperlinkHandler* hyperlinkHandler() {return definition_.hyperlinkHandler();}

        void setHyperlinkHandler(HyperlinkHandler* handler) {definition_.setHyperlinkHandler(handler);}
        
        void selectionToText(ArsLexis::String& out) const {definition_.selectionToText(out);}
        
        typedef Definition::RenderingProgressReporter RenderingProgressReporter;
        void setRenderingProgressReporter(RenderingProgressReporter* reporter) {definition_.setRenderingProgressReporter(reporter);}


	
    };
    
}

#endif