#ifndef ARSLEXIS_TEXT_RENDERER_HPP__
#define  ARSLEXIS_TEXT_RENDERER_HPP__

#include <FormGadget.hpp>
#include <Definition.hpp>

namespace ArsLexis 
{
    class TextRenderer: public FormGadget 
    {
        Definition definition_;
        RenderingPreferences& renderingPreferences_;
        ScrollBar* scrollBar_;
        WinHandle drawingWindow_;
        bool drawingWindowIsOffscreen_;
        Rectangle drawingWindowBounds_;
        Err lastRenderingError_;
        
        void checkDrawingWindow();
        
        void updateForm(Graphics& graphics);
        
        void doUpdateScrollbar();
        
        bool handleMouseEvent(const EventType& event);
        
    public:
 
        TextRenderer(Form& form, RenderingPreferences& prefs, ScrollBar* scrollBar = NULL);

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
        
        class RenderingErrorListener {
        public:
        
            virtual void handleRenderingError(TextRenderer& renderer, Err error) = 0;
            
            virtual ~RenderingErrorListener();
            
        };
        
        void setRenderingErrorListener(RenderingErrorListener* listener) {renderingErrorListener_ = listener;}
        
        enum ScrollbarUpdateOption {
            updateScrollbarNot,
            updateScrollbar
        };
        
        void scroll(WinDirectionType direction, uint_t items, ScrollbarUpdateOption update = updateScrollbar);
        
        void draw() {drawProxy();}
        
    protected:
    
        void drawProxy();

        void handleDraw(Graphics& graphics);
        
        bool handleEvent(EventType& event);
        
        bool handleEnter(const EventType& event);
        
    private:
        
        RenderingErrorListener* renderingErrorListener_;
	
    };
    
}

#endif