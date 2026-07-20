//
//  MiSTer PAL2 diagnostic logger — tagged lines in PAL.log for post-mortem.
//  Grep:  [DIAG]
//

#ifndef MISTER_DIAG_H
#define MISTER_DIAG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void PAL_Diag_Init(void);

/* Generic tagged line: [DIAG][tag] ... */
void PAL_Diag_Log(const char* tag, const char* fmt, ...);

/* Raw joy + mapped keys; logs on edges and Start+Select conflicts. */
void PAL_Diag_Input(uint32_t raw_joy, uint32_t held, uint32_t pressed, uint32_t released);

/* Video path: wait timeout / long wait / present rate. */
void PAL_Diag_VideoWait(int timed_out, uint32_t feedback, uint32_t expected_buf, int wait_iters);
void PAL_Diag_VideoPresent(int width, int height, int partial);
void PAL_Diag_VideoFlip(uint32_t frame_counter, int active_buf);

/* Lifecycle: bounce-to-title / quit paths. */
void PAL_Diag_Lifecycle(const char* event);
void PAL_Diag_Shutdown(const char* reason, int exit_code);
void PAL_Diag_Music(int track, int loop);

#ifdef __cplusplus
}
#endif

#endif
