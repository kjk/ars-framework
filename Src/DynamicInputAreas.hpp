/**
 * @file DynamicInputAreas.hpp
 * Generic interface to DynamicInputAreas features present on some new PalmOS 5 devices.
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__
#define __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__

#include "Debug.hpp"
#include <PalmOS.h>
#include <bitset>

namespace ArsLexis 
{

    class DIA_Support
    {
        enum Flag_t {
            flagHasPenInputMgr,
            flagHasSonySilkLib,
            flagLoadedSonySilkLib,
            flagSonyLibIsVsk,
            
            flagCount
        };
        
        typedef std::bitset<flagCount> Flags_t;
        Flags_t flags_;
        UInt16 sonySilkLibRefNum_;
        
        DIA_Support(const DIA_Support&);
        DIA_Support& operator=(const DIA_Support&);
        
        Boolean tryInitSonySilkLib() throw();
        void sonySilkLibDispose() throw();
        
    public:
        
        DIA_Support() throw();
        ~DIA_Support() throw();

        Boolean hasPenInputManager() const throw()
        {return flags_.test(flagHasPenInputMgr);}
        
        Boolean hasSonySilkLib() const throw()
        {return flags_.test(flagHasSonySilkLib);}
        
        Boolean available() const throw()
        {return hasPenInputManager()||hasSonySilkLib();}

        UInt32 notifyType() const throw()
        {return hasPenInputManager()?sysNotifyDisplayResizedEvent:sysNotifyDisplayChangeEvent;}        
        
        void handleNotify() const throw();
        
    };
    
}    

#endif