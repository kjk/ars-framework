/**
 * @file DynamicInputAreas.hpp
 * Generic interface to DynamicInputAreas features present on some new PalmOS 5 devices.
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__
#define __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__

#include "Debug.hpp"
#include "Library.hpp"

#include <PalmOS.h>

namespace ArsLexis 
{

    class Form;
    
    class DIA_Support
    {
        
        UInt16 hasPenInputMgr_:1;
        UInt16 hasSonySilkLib_:1;
        UInt16 sonyLibIsVsk_:1;
        UInt16 notUsed_:13;

        Library sonySilkLib_;
                        
        DIA_Support(const DIA_Support&);
        DIA_Support& operator=(const DIA_Support&);
        
        Boolean tryInitSonySilkLib();
        void sonySilkLibDispose();
        
    public:
        
        DIA_Support();
        ~DIA_Support();

        Boolean hasPenInputManager() const 
        {return hasPenInputMgr_;}
        
        Boolean hasSonySilkLib() const 
        {return hasSonySilkLib_;}
        
        Boolean available() const 
        {return hasPenInputManager()||hasSonySilkLib();}

        UInt32 notifyType() const 
        {return hasPenInputManager()?sysNotifyDisplayResizedEvent:sysNotifyDisplayChangeEvent;}        
       
        void handleNotify() const;
        
        Err configureForm(Form& form, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW) const;
        
    };
    
}    

#endif