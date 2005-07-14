#ifndef __BYTE_FORMAT_PARSER__
#define __ByTE_FORMAT_PARSER__

#include <DefinitionElement.hpp>
#include <IncrementalProcessor.hpp>

ulong_t readUnaligned32(const char* addr);
void    writeUnaligned32(char* addr, ulong_t value);

/**
 Used to change byteData to DefinitnionElements

 */
class ByteFormatParser: public BinaryIncrementalProcessor
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

        char**  styleNames_;
        ulong_t styleCount_;
        ulong_t totalStyleCount_;
        
        DefinitionElement* currentElement_;
        
        char*			inText_;
        ulong_t         inLength_;
        ulong_t			start_;
        bool                        finish_;
        
        status_t parse();
        
        bool parseElement();
        
        void parseElementParams();

        bool parseParam();
        
        bool parseHeader(const char* inText);
        
        DefinitionModel* model_;

    public:

        ByteFormatParser();

        ~ByteFormatParser();

        void reset();
       
        status_t handleIncrement(const char* payload, ulong_t& length, bool finish);

        /**
         * ignore length if inputTextLen = (UInt32)(-1)
         */
        status_t parseAll(const char* inputText, long inputTextLen);

        DefinitionModel* releaseModel();
        
 };

#endif
