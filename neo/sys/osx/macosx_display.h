#include "macosx_local.h"

@class NSDictionary;

extern NSDictionary *Sys_GetMatchingDisplayMode( glimpParms_t parms );

extern void Sys_StoreGammaTables();
extern void Sys_GetGammaTable(glwgamma_t *table);
extern void Sys_SetScreenFade(glwgamma_t *table, float fraction);

extern void Sys_FadeScreens();
extern void Sys_FadeScreen(CGDirectDisplayID display);
extern void Sys_UnfadeScreens();
extern void Sys_UnfadeScreen(CGDirectDisplayID display, glwgamma_t *table);
extern void Sys_ReleaseAllDisplays();

