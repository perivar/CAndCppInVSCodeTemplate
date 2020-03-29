#define DllExport __declspec(dllexport)

//------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif
    bool DllExport InitDll() ///< must be called from host right after loading dll
    {
        return true;
    }
    bool DllExport ExitDll() ///< must be called from host right before unloading dll
    {
        return true;
    }
    int DllExport GetPluginFactory(int a, int b)
    {
        return (a + b);
    }
#ifdef __cplusplus
} // extern "C"
#endif
