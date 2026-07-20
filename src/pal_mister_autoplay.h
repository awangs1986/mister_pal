//
//  MiSTer autoplay: New Game â†?advance dialog â†?capture on ã€Œçš®ç—’ã€?â†?freeze
//

#ifndef PAL_MISTER_AUTOPLAY_H
#define PAL_MISTER_AUTOPLAY_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

void PAL_MisterAuto_SetEnabled(int on);
int  PAL_MisterAuto_Enabled(void);

/* Call from PAL_ShowDialogText â€?wide dialog line. */
void PAL_MisterAuto_OnDialogText(const wchar_t* text);

/* If autoplay: inject Search and return non-zero to skip waiting. */
int  PAL_MisterAuto_ShouldSkipWait(void);

/* Opening menu: force New Game (slot 0). */
int  PAL_MisterAuto_ForceNewGame(void);

/* Skip splash / AVI when autoplay. */
int  PAL_MisterAuto_ShouldSkipMedia(void);

#ifdef __cplusplus
}
#endif

#endif
