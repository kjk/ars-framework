/**
 * @file Application.hpp
 * Interface of Application class. 
 * 
 * @note you <b>must</b> define function @c Application::instance() in your own module. 
 * The obvious implementation will look like this:
 * @code
 * namespace ArsLexis {
 *     Application& Application::instance() {
 *         return getInstance(appFileCreator);
 *     }
 * }
 * @endcode
 * The easiest way to do so is to use @c IMPLEMENT_APPLICATION_INSTANCE(appFileCreator) macro.
 */

#ifndef __ARSLEXIS_APPLICATION_HPP__
#define __ARSLEXIS_APPLICATION_HPP__

#include <Debug.hpp>
#include <Utility.hpp>
#include <list>

#ifdef __MWERKS__
#include <CWCallbackThunks.h>
#endif // __MWERKS__

 
namespace ArsLexis 
{

    class Form;

    /**
     * The core of application.
     * Acts as a container for globally accessible variables and services.
     * Should be used only by subclassing. To ensure proper application control flow 
     * the proper way to use @c Application class should be through call to 
     * @c Application::main(), passing actual subclass as template argument.
     * Each subclass should declare the following static variables: 
     *  * static const UInt32 requiredRomVersion
     *  * static const UInt32 creatorId=appFileCreator;
     *  * static const UInt16 notEnoughMemoryAlertId;
     * They are required for proper working of @c Appliation::main().
     */
    class Application: private NonCopyable
    {

        /**
         * @internal 
         * Type used to store @c Form objects through application's lifetime.
         */
        typedef std::list<Form*> Forms_t;
        
        /**
         * @internal 
         * Stores form id -> @c Form mappings so that we can access @c Form objects from within event handlers.
         */
        Forms_t forms_;

        /**
         * @internal 
         * Adds to internal store. Used internally by @c Form class constructor.
         */
        void registerForm(Form& form);
        
        /**
         * @internal 
         * Removes form from store. Used internally by @c Form class destructor.
         */
        void unregisterForm(Form& form);
        
        /**
         * @internal 
         * Called internally by @c Application(UInt32).
         */
        static Err setInstance(UInt32 creatorId, Application* app) throw();
        
        /**
         * @internal
         * Called internally by @c instance().
         */
        static Application* getInstance(UInt32 creatorId) throw();
        
        /**
         * @internal 
         * Actual @c EvtGetEvent() timeout used by @c runEventLoop().
         */
        Int32 eventTimeout_;

#ifdef __MWERKS__        
        /**
         * @internal
         * Wraps @c Form::routeEventToForm() for use in expanded mode.
         */
        _CW_EventHandlerThunk formEventHandlerThunk_;
#endif
        
        /**
         * @internal 
         * Stores card number this application's database resides on.
         */
        UInt16 cardNo_;
        
        /**
         * @internal 
         * Stores id of this app's database.
         */
        LocalID databaseId_;

        /**
         * @internal
         * Stores creator's id through application's lifetime.
         */
        const UInt32 creatorId_;
        
        UInt32 romVersion_;
        
        static void sendEvent(UInt16 eventId, const void* eventData, UInt16 dataLength) throw();
        
    protected:
    
#ifdef appFileCreator    
        explicit Application(UInt32 creatorId=appFileCreator);
#else
        /**
         * Constructor. 
         * Initializes @c Application object and sets it as current instance through the call to @c FtrSet().
         */
        explicit Application(UInt32 creatorId);
#endif

        /**
         * Checks if the ROM version is high enough to run application.
         * @param requiredVersion version needed to run this application.
         * @param alertId if version is lower than @c requiredVersion, this alert will be shown.
         */
        static Err checkRomVersion(UInt32 requiredVersion, UInt16 launchFlags, UInt16 alertId=frmInvalidObjectId);
        
        /**
         * Creates and activates a new @c Form object.
         * Called in response to @c frmLoadEvent. 
         * Override the default implementation to use subclasses of @c Form class.
         * @param formId id of form to load.
         */
        virtual void loadForm(UInt16 formId);
        
        virtual Form* createForm(UInt16 formId);
        
        /**
         * Called from within @c runEventLoop() to handle application-level events.
         * Currently the only handled event is @c frmLoadEvent. Override to add support
         * for other kinds of events.
         * @param event The event to handle.
         * @return @c true if @c event is fully processed, @c false otherwise.
         */
        virtual bool handleApplicationEvent(EventType& event);
        
        /**
         * Waits for next event.
         * Allows to use non-standard function to obtain event, which might be useful
         * in network apps, that need to remain responsive while processing requests.
         * Current implementation simply calls @c EvtGetEvent() with actual @c eventTimeout_.
         * @param event @c EventType structure to be filled on return.
         */
        virtual void waitForEvent(EventType& event);
        
        /**
         * Runs event-processing loop.
         * Loop is terminated when @c appStopEvent is detected.
         */
        virtual void runEventLoop();
        
        Err registerNotify(UInt32 notifyType, Int8 priority=sysNotifyNormalPriority, void* userData=0);

        Err unregisterNotify(UInt32 notifyType, Int8 priority=sysNotifyNormalPriority);
        
        /**
         * Called from @c handleLaunchCode() in case when application is run with @c sysAppLaunchCmdNotify code.
         * Override it to perform some useful stuff.
         */
        virtual Err handleSystemNotify(SysNotifyParamType&)
        {return errNone;}
        
        /**
         * Called from @c handleLaunchCode() in case when application is run with @c sysAppLaunchCmdNormalLaunch code.
         * You should override it and provide some reasonable contents like this (error checking omitted for clarity):
         * @code
         *     checkRomVersion(myRequiredRomVersion, myRomIncompatibleAlert);
         *     gotoForm(myMainForm);
         *     runEventLoop();
         * @endcode
         */ 
        virtual Err normalLaunch()
        {return errNone;}
        
        void setEventTimeout(Int32 timeout)
        {eventTimeout_=timeout;}
        
    public:
    
        /**
         * Enumerates used feature indexes.
         * Subclasses should not assume that next available feature index is some fixed number,
         * but declare it's first feature as 
         * <code>featureSomeSubclassFeature=Application::featureFirstAvailable</code>.
         */
        enum Feature 
        {
            /**
             * Feature index that is used to store pointer to current @c Application instance.
             */
            featureInstancePointer,
            
            /**
             * Used to store pointer to current @c RootLogger.
             */
            featureRootLoggerPointer,
            
            /**
             * First available feature index for use in subclasses.
             */
            featureFirstAvailable
        };
        
/*
        template<class Predicate>
        Form* getOpenForm(const Predicate& pred) const
        {
            Form* result=0;
            Forms_t::const_iterator it=std::find_if(forms_.begin(), forms_.end(), pred);
            if (it!=forms_.end())
                result=*it;
            return result;
        }
*/

        Form* getOpenForm(UInt16 id) const;
        Form* getOpenForm(WinHandle winHandle) const;

        Form* getOpenForm(FormType* form) const
        {return getOpenForm(FrmGetWindowHandle(form));}
        
        virtual Err handleLaunchCode(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

        /**
         * @c Application object's "virtual constructor". Here not-trivial initializations of members
         * should take place.
         * @return error code, if initialization required for normal work failed. Returning error code
         * different from @c errNone here prevents application from being run.
         */
        virtual Err initialize();
    
        /**
         * Destructor. Unregisters current instance.
         */
        virtual ~Application();
        
        virtual void abort()
        {delete this;}

        /**
         * Gets currently registered @c Application instance.
         */
        static Application& instance();
        
        static UInt32 creator();
        
        UInt16 cardNumber() const
        {return cardNo_;}
        
        LocalID databaseId() const
        {return databaseId_;}
            
        friend class Form;

        /**
         * Instantiates object of @c AppClass class, calls its @c handleLaunchCode(UInt16, MemPtr, UInt16) function and deletes object afterwards.
         * @return standard PalmOS error code.
         * @note This function is designed so that it should be the only code called from @c PilotMain() function,
         * unless you know what you're doing.
         */
        template<class AppClass, UInt16 alertId> 
        static Err main(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
        
        static void gotoForm(UInt16 formId)
        {FrmGotoForm(formId);}
        
        static void popupForm(UInt16 formId)
        {FrmPopupForm(formId);}
        
        UInt32 romVersion() const
        {return romVersion_;}
        
        UInt16 romVersionMajor() const
        {return sysGetROMVerMajor(romVersion());}
        
        UInt16 romVersionMinor() const
        {return sysGetROMVerMinor(romVersion());}
        
        friend void handleBadAlloc();
        
        enum Error
        {
            errFirstAvailable=appErrorClass
        };
        
        enum Event
        {
            appFirstAvailableEvent=firstUserEvent
        };

        template<typename Data>
        static void sendEvent(UInt16 eventId, const Data& data)
        {sendEvent(eventId, &data, sizeof(data));}
        
        static void sendEvent(UInt16 eventId)
        {sendEvent(eventId, 0, 0);}
        
        friend void handleBadAlloc();
    };
    
    template<class AppClass, UInt16 alertId> 
    Err Application::main(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
    {
        Err error=Application::checkRomVersion(AppClass::requiredRomVersion, launchFlags, alertId);
        if (!error)
        {
            AppClass* volatile app=static_cast<AppClass*>(getInstance(AppClass::creatorId));
            if (app)
            {
                ErrTry {
                    error=app->handleLaunchCode(cmd, cmdPBP, launchFlags);
                } 
                ErrCatch (ex) {
                    error=ex;
                } ErrEndCatch
            }
            else
            {
                if (0 == (launchFlags & sysAppLaunchFlagNewGlobals))
#ifdef __MWERKS__                
                    error=_CW_SetupExpandedMode();
#else
                    error=memErrNoStore;
#endif // __MWERKS__                                        
                if (!error)
                {
                    ErrTry {
                        app=new AppClass;
                        error=app->initialize();
                        if (!error)
                            error=app->handleLaunchCode(cmd, cmdPBP, launchFlags);
                    }
                    ErrCatch (ex) {
                        error=ex;
                    } ErrEndCatch
                    delete app;
                    if (memErrNotEnoughSpace==error)
                        FrmAlert(AppClass::notEnoughMemoryAlertId);
                }
            }
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
    Application& Application::instance() { \
        Application* app=getInstance((creatorId)); \
        assert(app!=0); \
        return  *app;\
    } \
    \
    UInt32 Application::creator() {\
        return creatorId; \
    }\
} 

#endif