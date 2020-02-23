#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"

using namespace Steinberg;

FUnknown *gStandardPluginContext = 0;

#if WINDOWS

extern "C"
{
    typedef bool(PLUGIN_API *InitModuleProc)();
    typedef bool(PLUGIN_API *ExitModuleProc)();
}
static const FIDString kInitModuleProcName = "InitDll";
static const FIDString kExitModuleProcName = "ExitDll";

#endif

static int verbose = 0;

void handleError(char *text, tresult result)
{
    printf("\t%s: [0x%.8X] ",
           text, result);

    switch (result)
    {
    case kNoInterface:
        printf("NoInterface");
        break;
    case kResultOk:
        printf("ResultOk");
        break;
    case kResultFalse:
        printf("ResultFalse");
        break;
    case kInvalidArgument:
        printf("InvalidArgument");
        break;
    case kNotImplemented:
        printf("NotImplemented");
        break;
    case kInternalError:
        printf("InternalError");
        break;
    case kNotInitialized:
        printf("NotInitialized");
        break;
    case kOutOfMemory:
        printf("OutOfMemory");
        break;
    default:
        printf("Error text not found");
        break;
    }
    printf("\n\n");
}

extern int
main(int argc, char *argv[])
{
    HMODULE hModule = NULL;
    Steinberg::tresult result = 0;
    int rc = 0;
    char *path;

    strcpy(path, argv[1]);

    printf("* Loading module...\n\n\t%s\n\n", path);
    hModule = LoadLibraryA(path);
    if (hModule)
    {
        InitModuleProc initProc = (InitModuleProc)GetProcAddress(hModule, kInitModuleProcName);
        if (initProc)
        {
            if (initProc() == false)
            {
                FreeLibrary(hModule);
                return false;
            }
        }
        GetFactoryProc proc = (GetFactoryProc)GetProcAddress(hModule, "GetPluginFactory");
        IPluginFactory *factory = proc ? proc() : 0;
        if (factory)
        {
            Steinberg::PFactoryInfo factoryInfo;
            factory->getFactoryInfo(&factoryInfo);
            printf("  Factory Info:\n\tvendor = %s\n\turl = %s\n\temail = %s\n\n",
                   factoryInfo.vendor, factoryInfo.url, factoryInfo.email);

            //---print all included plugins---------------
            int32 i;
            for (i = 0; i < factory->countClasses(); i++)
            {
                Steinberg::PClassInfo classInfo;
                result = factory->getClassInfo(i, &classInfo);
                if (result != kResultTrue)
                {
                    // TODO: error handling
                    return 1;
                }

                printf("  Class Info %d:\n\tname = %s\n\tcategory = %s\n\tcid = %.*s\n\n",
                       i, classInfo.name, classInfo.category, 16, classInfo.cid);

                // info.category will be kVstAudioEffectClass for Processor component.
                // skip this component if not.
                if (strcmp(classInfo.category, kVstAudioEffectClass) != 0)
                {
                    continue;
                }

                Vst::IComponent *component(nullptr);
                result = factory->createInstance(classInfo.cid,       // tell factory which plugin to be created.
                                                 Vst::IComponent_iid, // tell factory which type of interface you want.
                                                 (void **)&component  // get the pointer to `comp`, and pass it as (void **)
                );
                if (result != kResultTrue)
                {
                    // TODO: error handling
                    handleError("Creating process instance failed", result);
                    return 1;
                }

                // now `component` shall be valid pointer of Vst::IComponent.

                // initialize comp
                result = component->setIoMode(Vst::IoModes::kAdvanced);
                if (result != kResultTrue)
                {
                    handleError("Setting IO Mode failed", result);
                    // return 1;
                }

                // you should define host context object before and pass it here as `FUnknown *`.
                // the host context object is the class which normally derives Vst::IHostApplication,
                // Vst::IComponentHandler, Vst::IPluginInterfaceSupport, etc.
                result = component->initialize(gStandardPluginContext);
                if (result != kResultTrue)
                {
                    handleError("Initializing component failed", result);
                    // return 1;
                }

                TUID edit_cid;
                component->getControllerClassId(edit_cid);
                // (in detail, IEditController interface may be obtained from IComponent directly if the plugin
                //  derives SingleComponentEffect.
                //  For such plugins, do not use this method and obtain IEditController with `comp->queryInstance()`
                // )

                printf("\tController Class Id = %.*s\n\n",
                       16, edit_cid);

                Vst::IEditController *edit(nullptr);
                result = factory->createInstance(edit_cid,
                                                 Vst::IEditController_iid,
                                                 (void **)&edit);
                if (result != kResultTrue)
                {
                    handleError("Creating edit instance failed", result);
                    // return 1;
                }

                // initialize the EditController component too.
                result = edit->initialize(gStandardPluginContext);
                if (result != kResultTrue)
                {
                    handleError("Initializing edit failed", result);
                    // return 1;
                }

                //...

                // now the two components are created.
                // connect and setup them.

                // use the plugin.

                // ...

                // don't forget destruction after using it.
                edit->terminate();
                component->terminate();

                edit->release();
                component->release();
            }

            factory->release();
        }

        ExitModuleProc exitProc = (ExitModuleProc)GetProcAddress(hModule, kExitModuleProcName);
        if (exitProc)
            exitProc();
        FreeLibrary(hModule);
    }

    printf("Press Enter. ");
    static char readBuf[256];
    gets(readBuf);

    return rc;
}