#include <windows.h>

#include "sc2kfix.h"
#include "neomaxis/version.h"
#include "neomaxis/version.h"

namespace {

sc2kfix_mod_hook_t g_mod_hooks[] = {
    { "Hook_SimCalendarAdvance_Before", 0 },
    { "Hook_OnNewCity_Before", 0 },
    { "L_SimcityApp_DoLoad_Before", 0 },
    { "L_SimcityApp_DoLoad_After", 0 },
    { "L_SimcityApp_DoLoad_Result", 0 },
    { "L_SimcityApp_DoSave_Before", 0 },
    { "L_SimcityApp_DoSave_After", 0 }
};

sc2kfix_mod_info_t g_mod_info = {
    1,

    NEOMAXIS_VERSION_MAJOR,
    NEOMAXIS_VERSION_MINOR,
    NEOMAXIS_VERSION_PATCH,

    0, 11, 0,

    NEOMAXIS_MOD_NAME,
    NEOMAXIS_MOD_SHORT_NAME,
    NEOMAXIS_MOD_AUTHOR,
    NEOMAXIS_MOD_DESCRIPTION,

    7,
    g_mod_hooks
};

}

HOOKCB sc2kfix_mod_info_t* HookCb_GetModInfo(void)
{
    return &g_mod_info;
}
