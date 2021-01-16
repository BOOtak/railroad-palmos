#include <PalmCompatibility.h>
#include <PalmOS.h>
#include <StdIOPalm.h>

// MathLib should be included after Palm OS headers
#include <MathLib.h>

#include "game.h"

#define DRAW_FPS 1

void app_handle_event(EventPtr event) {}

void StopMath() {
  Err error;
  UInt usecount;

  error = MathLibClose(MathLibRef, &usecount);
  ErrFatalDisplayIf(error, "Can't close MathLib");
  if (usecount == 0) SysLibRemove(MathLibRef);
}

void StopApplication() {
  StopGame();
  StopMath();
}

void InitMath() {
  Err error;

  // InitGame math lib
  error = SysLibFind(MathLibName, &MathLibRef);
  if (error) error = SysLibLoad(LibType, MathLibCreator, &MathLibRef);
  ErrFatalDisplayIf(error, "Can't find MathLib");
  error = MathLibOpen(MathLibRef, MathLibVersion);
  ErrFatalDisplayIf(error, "Can't open MathLib");
}

void StartApplication() {
  EventType event;
  UInt32 ts = TimGetTicks();
  UInt32 ts0 = ts;
  UInt32 ts1;
  UInt32 tsN;
  Int16 updateRes = 0;
  Int32 delay = evtNoWait;
  UInt16 frameDelay = SysTicksPerSecond() / FPS;
#ifdef DRAW_FPS
  char fps_buf[255];
  char update_buf[255];
  UInt32 tick_counter = 0;
  UInt16 frame_counter = 0;
  UInt32 TPS = SysTicksPerSecond();
  Int16 fpsStrLen;
#endif

  // double buffering
  UInt16 error;
  WinHandle originalWindow, offscreenWindow;
  RectangleType bounds;
  Coord width, height;
  WinGetDisplayExtent(&width, &height);
  offscreenWindow = WinCreateOffscreenWindow(width, height, genericFormat, &error);
  if (error != 0) {
    return;
  }

  originalWindow = WinSetDrawWindow(offscreenWindow);
  WinGetDrawWindowBounds(&bounds);
  WinSetDrawWindow(originalWindow);

  InitMath();
  InitGame();

  do {
    WinSetDrawWindow(offscreenWindow);
    ts1 = TimGetTicks();
    if (updateRes == 0) {
      updateRes = Update(ts1 - ts);
      tsN = TimGetTicks();
      delay = frameDelay - (tsN - ts1);
      if (delay < 0) {
        delay = 0;
      }
    } else {
      StrPrintF(update_buf, "%ld sec", (tsN - ts0) / SysTicksPerSecond());
      WinDrawChars(update_buf, StrLen(update_buf), 50, 0);
    }
#ifdef DRAW_FPS
    tick_counter += ts1 - ts;
    frame_counter++;
    if (tick_counter > TPS) {
      fpsStrLen = StrPrintF(fps_buf, "%d FPS", frame_counter);
      tick_counter = 0;
      frame_counter = 0;
      WinDrawChars(fps_buf, fpsStrLen, 0, 0);
    }
#endif
    ts = ts1;
    WinCopyRectangle(offscreenWindow, originalWindow, &bounds, 0, 0, winPaint);
    WinSetDrawWindow(originalWindow);

    EvtGetEvent(&event, delay);
    if (!SysHandleEvent(&event)) {
      app_handle_event(&event);
    }
  } while (event.eType != appStopEvent);

  WinDeleteWindow(offscreenWindow, 1);
}

UInt32 PilotMain(UInt16 cmd, void* cmdPBP, UInt16 launchFlags) {
  switch (cmd) {
    // Normal application launch
    case sysAppLaunchCmdNormalLaunch: {
      StartApplication();
      StopApplication();
      break;
    }
  }
  return 0;
}
