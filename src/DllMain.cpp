#include <Windows.h>
#include <ZenGin/zGothicAPI.h>

BOOL WINAPI DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;

#if defined(G1_TEXTURE_TRACE_PLUGIN)
    return GetGameVersion() == Engine_G1;
#elif defined(IAI_PLUGIN) || defined(MINI_EXT_ITEM_VIEW_PLUGIN)
    const int engine = GetGameVersion();
    return engine == Engine_G1 || engine == Engine_G1A || engine == Engine_G2 || engine == Engine_G2A;
#else
    return GetGameVersion() == Engine_G2A;
#endif
}