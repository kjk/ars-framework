#ifndef __BYTE_FORMAT_PARSER__
#define __ByTE_FORMAT_PARSER__

#include <DefinitionElement.hpp>

ulong_t readUnaligned32(const char* addr);
void    writeUnaligned32(char* addr, ulong_t value);

/**
 Used to change byteData to DefinitnionElements

 */
class ByteFormatParser
{
    private:
        typedef std::vector<DefinitionElement*> ElementsPointers_t;
        ElementsPointers_t stack_;
    
        typedef ulong_t elementTypeType;
        typedef ulong_t elementSizeType;
    
        elementTypeType currentElementType_;
        elementSizeType currentElementParamsLength_;
        elementTypeType currentParamType_;
        elementSizeType currentParamLength_;
        
        ulong_t totalElementsCount_;
        ulong_t elementsCount_;
        ulong_t version_;
        elementSizeType totalSize_;
        bool    headerParsed_;
        
        DefinitionElement* currentElement_;
        
        ArsLexis::String            inText_;
        ulong_t                     inLength_;
        ArsLexis::String::size_type start_;
        bool                        finish_;

        ArsLexis::status_t parse();
        
        bool parseElement();
        
        void parseElementParams();

        bool parseParam();
        
        bool parseHeader(const ArsLexis::char_t* inText);
        
    public:

        Definition::Elements_t  elems;

        ByteFormatParser();

        ~ByteFormatParser();

        void reset();
       
        ArsLexis::status_t handleIncrement(const ArsLexis::char_t* text, ulong_t length, bool finish=false);

        /**
         * ignore length if inputTextLen = (UInt32)(-1)
         */
        ArsLexis::status_t parseAll(const ArsLexis::char_t*  inputText, UInt32 inputTextLen);

        void replaceElements(Definition::Elements_t& el);
};

#endif
