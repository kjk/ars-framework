/**
 * @file DynamicInputAreas.hpp
 * Generic interface to DynamicInputAreas features present on some new PalmOS 5 devices.
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__
#define __ARSLEXIS_DYNAMIC_INPUT_AREAS_HPP__

#include <Debug.hpp>
#include <Library.hpp>

#include <PalmOS.h>

#ifndef SetBits
#define SetBits( b, len )      ( ( ( 1U << ( ( len ) - 1 ) ) - 1U + ( 1U << ( ( len ) - 1 ) ) ) << ( b ) )
#endif

#ifndef pinMaxConstraintSize
#define pinMaxConstraintSize 	SetBits( 0, ( sizeof( Coord) * 8 ) - 1 )
#endif

class Form;

class DIA_Support: private NonCopyable
{
    
    UInt16 hasPenInputMgr_:1;
    UInt16 hasSonySilkLib_:1;
    UInt16 sonyLibIsVsk_:1;

    Library sonySilkLib_;
                    
    DIA_Support(const DIA_Support&) throw();
    DIA_Support& operator=(const DIA_Support&) throw();
    
    bool tryInitSonySilkLib() throw();
    void sonySilkLibDispose() throw();
    
public:
    
    DIA_Support() throw();
    ~DIA_Support() throw();

    bool hasPenInputManager() const  throw()
    {return hasPenInputMgr_;}
    
    bool hasSonySilkLib() const  throw()
    {return hasSonySilkLib_;}
    
    bool available() const  throw()
    {return hasPenInputManager()||hasSonySilkLib();}
    
    operator bool() const
    {return available();}

    UInt32 notifyType() const  throw()
    {return hasPenInputManager()?sysNotifyDisplayResizedEvent:sysNotifyDisplayChangeEvent;}        
   
    void handleNotify() const throw();
    
    Err configureForm(Form& form, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW, bool disableTrigger=false) const throw();
    
};

#endif