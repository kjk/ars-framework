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
#include <bitset>

namespace ArsLexis 
{

    class DIA_Support
    {
        enum Flag_t {
            flagHasPenInputMgr,
            flagHasSonySilkLib,
            flagSonyLibIsVsk,

            flagCount
        };
        
        typedef std::bitset<flagCount> Flags_t;
        Flags_t flags_;
        
        Library sonySilkLib_;
                        
        DIA_Support(const DIA_Support&);
        DIA_Support& operator=(const DIA_Support&);
        
        Boolean tryInitSonySilkLib();
        void sonySilkLibDispose();
        
    public:
        
        DIA_Support();
        ~DIA_Support();

        Boolean hasPenInputManager() const 
        {return flags_.test(flagHasPenInputMgr);}
        
        Boolean hasSonySilkLib() const 
        {return flags_.test(flagHasSonySilkLib);}
        
        Boolean available() const 
        {return hasPenInputManager()||hasSonySilkLib();}

        UInt32 notifyType() const 
        {return hasPenInputManager()?sysNotifyDisplayResizedEvent:sysNotifyDisplayChangeEvent;}        
        
        void handleNotify() const;
        
    };
    
}    

#endif