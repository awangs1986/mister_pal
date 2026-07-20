//
//  UTIL_Platform_* for MiSTer hybrid (replaces unix.cpp FLTK/syslog).
//

#include "common.h"
#include "util.h"
#include "palcfg.h"

#include <stdio.h>
#include <string.h>

BOOL
UTIL_GetScreenSize(
   DWORD *pdwScreenWidth,
   DWORD *pdwScreenHeight
)
{
   if (!pdwScreenWidth || !pdwScreenHeight)
      return FALSE;
   *pdwScreenWidth = 320;
   *pdwScreenHeight = 200;
   return TRUE;
}

BOOL
UTIL_IsAbsolutePath(
   LPCSTR lpszFileName
)
{
   return lpszFileName && lpszFileName[0] == '/';
}

static void
mister_log_cb(LOGLEVEL level, const char* str, const char* unused)
{
   (void)level;
   (void)unused;
   if (str)
      fputs(str, stderr);
}

INT
UTIL_Platform_Init(
   int argc,
   char* argv[]
)
{
   (void)argc;
   (void)argv;
   gConfig.fLaunchSetting = FALSE;
   /* Release PAL_DEFAULT_LOGLEVEL is FATAL-only; allow INFO for bring-up logs. */
   gConfig.iLogLevel = LOGLEVEL_INFO;
   UTIL_LogAddOutputCallback(mister_log_cb, LOGLEVEL_INFO);
   return 0;
}

VOID
UTIL_Platform_Quit(
   VOID
)
{
}
