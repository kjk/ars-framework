/**
 * @file Application.hpp
 * Interface of Application class. 
 * 
 * @note you <b>must</b> define function @c Application::instance() in your own module. 
 * The obvious implementation will look like this:
 * @code
 * namespace ArsLexis {
 *     Application& Application::instance() throw() {
 *         return getInstance(appFileCreator);
 *     }
 * }
 * @endcode
 * The easiest way to do so is to use @c IMPLEMENT_APPLICATION_INSTANCE(appFileCreator) macro.
 */

#ifndef __ARSLEXIS_APPLICATION_HPP__
#define __ARSLEXIS_APPLICATION_HPP__

#include "Debug.hpp"

#include <CWPalmOS.h>
#include <CWCallbackThunks.h>

#include <map>
#include <memory>


namespace ArsLexis 
{

    class Form;

    /**
     * The core of application.
     * Acts as a container for globally accessible variables and services.
     * Should be used only by subclassing. To ensure proper application control flow 
     * the proper way to use @c Application class should be through call to 
     * @c Application::main(), passing actual subclass as template argument.
     */
    class Application
    {
        /**
         * @internal
         * Undefined. Prevents copying.
         */
        Application(const Application&);

        /**
         * @internal
         * Undefined. Prevents copying.
         */
        Application& operator=(const Application&);
        
        /**
         * @internal 
         * Type used to map form identifiers to concrete @c Form objects.
         */
        typedef std::map<UInt16, Form*> Forms_t;
        
        /**
         * @internal 
         * Stores form id -> @c Form mappings so that we can access @c Form objects from within event handlers.
         */
        Forms_t forms_;
        
        /**
         * @internal 
         * Adds form id -> @c Form mapping. Used internally by @c Form class constructor.
         * @param id form id.
         * @param form pointer to associated @c Form object.
         */
        void registerForm(UInt16 id, Form* form);
        
        /**
         * @internal 
         * Removes form id -> @c Form mapping. Used internally by @c Form class destructor.
         */
        void unregisterForm(UInt16 id);
        
        /**
         * @internal 
         * Called internally by @c Application(UInt32).
         * @throws std::bad_alloc in case system runs out of memory.
         */
        static void setInstance(UInt32 creatorId, Application* app) throw(std::bad_alloc);
        
        /**
         * @internal
         * Called internally by @c instance().
         */
        static Application& getInstance(UInt32 creatorId) throw();
        
        Int32 eventTimeout_;
        const UInt16 launchCommand_;
        MemPtr const launchParameterBlock_;
        const UInt16 launchFlags_;
        _CW_EventHandlerThunk formEventHandlerThunk_;
        UInt16 cardNo_;
        LocalID databaseId_;

        /**
         * @internal
         * Stores creator's id through application's lifetime.
         */
        const UInt32 creatorId_;
        
    protected:
    
        /**
         * Enumerates used feature indexes.
         * Subclasses should not assume that next available feature index is some fixed number,
         * but declare it's first feature as 
         * <code>featureSomeSubclassFeature=Application::featureFirstAvailable</code>.
         */
        enum Feature 
        {
            featureInstancePointer,
            featureFirstAvailable
        };
    
#ifdef appFileCreator    
        explicit Application(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags, UInt32 creatorId=appFileCreator) throw(std::bad_alloc, PalmOSError);
#else
        /**
         * Constructor. 
         * Initializes @c Application object and sets it as current instance through the call to @c FtrSet().
         */
        explicit Application(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags, UInt32 creatorId) throw(std::bad_alloc, PalmOSError);
#endif

        /**
         * Checks if the ROM version is high enough to run application.
         * @param requiredVersion version needed to run this application.
         * @param alertId if version is lower than @c requiredVersion, this alert will be shown.
         * @exception PalmOSError is thrown if version is too low (with code @c sysErrRomIncompatible).
         */
        void checkRomVersion(UInt32 requiredVersion, UInt16 alertId=frmInvalidObjectId) throw(PalmOSError);
        
        /**
         * Creates and activates a new @c Form object.
         * Called in response to @c frmLoadEvent. 
         * Override the default implementation to use subclasses of @c Form class.
         * @param formId id of form to load.
         */
        virtual void loadForm(UInt16 formId);
        
        /**
         * Called from within @c runEventLoop() to handle application-level events.
         * Currently the only handled event is @c frmLoadEvent. Override to add support
         * for other kinds of events.
         * @param event The event to handle.
         * @return @c true if @c event is fully processed, @c false otherwise.
         */
        virtual Boolean handleApplicationEvent(EventType& event);
        
        /**
         * Runs event-processing loop.
         * Loop is terminated when @c appStopEvent is detected.
         */
        virtual void runEventLoop();
        
        Form* getOpenForm(UInt16 id) const throw();
        
        void registerNotify(UInt32 notifyType, Int8 priority=sysNotifyNormalPriority, void* userData=0) throw(PalmOSError);
        void unregisterNotify(UInt32 notifyType, Int8 priority=sysNotifyNormalPriority) throw(PalmOSError);

    public:
    
        /**
         * Destructor. Unregisters current instance.
         */
        virtual ~Application() throw();

        /**
         * Gets currently registered @c Application instance.
         */
        static Application& instance() throw();
        
        
        UInt16 launchCommand() const throw() 
        {return launchCommand_;}
        
        MemPtr launchParameterBlock() const throw()
        {return launchParameterBlock_;}
        
        UInt16 launchFlags() const throw()
        {return launchFlags_;}
        
        UInt16 cardNumber() const throw()
        {return cardNo_;}
        
        LocalID databaseId() const throw()
        {return databaseId_;}
            
        friend class Form;

        /**
         * Placeholder for main application loop.
         * You must override it in your subclass..
         * The typical implementation will look like this:
         * @code
         *     checkRomVersion(myRomVersion, myRomIncompatibleAlert);
         *     gotoForm(myMainForm);
         *     runEventLoop();
         * @endcode
         */
        virtual void run()=0;
        
        /**
         * Instantiates object of @c AppClass class, calls its @c run() function and deletes object afterwards.
         * This function will catch @c PalmOSError or @c std::bad_alloc exceptions, perform some
         * cleanup and return appropriate error code.
         * @return standard PalmOS error code.
         * @note This function is designed so that it should be the only code called from @c PilotMain() function,
         * unless you know what you're doing.
         */
        template<class AppClass> 
        static Err main(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
        
        void gotoForm(UInt16 formId) throw()
        {FrmGotoForm(formId);}
        
        void popupForm(UInt16 formId) throw()
        {FrmPopupForm(formId);}

    };
    
    template<class AppClass> 
    Err Application::main(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
    {
        Err error=errNone;
        if (0 == (launchFlags & sysAppLaunchFlagNewGlobals))
            error=_CW_SetupExpandedMode(); // This makes it possible to use polymorphism, exceptions and all the other stuff that makes C++ rules.
        if (!error)
        {
            AppClass* application=0;
            try {            
                application=new AppClass(cmd, cmdPBP, launchFlags);
                application->run();
            }
            catch (const std::bad_alloc& ex)
            {
                error=memErrNotEnoughSpace;
            }
            catch (const PalmOSError& ex)
            {
                error=ex.code();
            }
            delete application;
        }
        return error;
    }  
    
}

/**
 * Implements @c ArsLexis::Application::instance() function. 
 * This macro should be used in module that implements actual @c ArsLexis::Application subclass.
 * @note remember not to put semicolon after this macro, otherwise you'll get some pretty misleading error message.
 * @param creatorId application creator id. In most apps this would be stationery-generated @c appFileCreator.
 */
#define IMPLEMENT_APPLICATION_INSTANCE(creatorId) \
namespace ArsLexis { \
    Application& Application::instance() throw() { \
        return getInstance((creatorId)); \
    } \
} 

#endif