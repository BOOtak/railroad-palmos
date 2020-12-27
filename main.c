#include <PalmCompatibility.h>
#include <PalmOS.h>
#include <StdIOPalm.h>

#define OLOLO  // keep it here not to mix MathLib with other includes on code reformat
#include <MathLib.h>

#include "tables.h"

#define W3D 2
#define H3D 2

#define SCREEN_W 160
#define SCREEN_H 160

#define DRAW_FPS 1
#define FPS 30

// 1 bit - sign; 11 bits - integer part; 4 bits - fraction part
#define FRAC_BITS 12
#define SIGN_MASK 0xFFF00000
#define FRAC_MASK 0x00000FFF
#define MANT_MASK 0xFFFFF000

#define FIX_MUL(X, Y) ((Fixed)(((Int32)((X) * (Y))) >> FRAC_BITS))
#define FIX_DIV(X, Y) ((Fixed)(((Int32)(X) << FRAC_BITS) / Y))
#define TO_INT(X) ((X) >> FRAC_BITS)
#define FLOAT_TO_FIX(X) ((Fixed)((X) * (1 << FRAC_BITS)))
#define INT_TO_FIX(X) ((Fixed)((X) << FRAC_BITS))

typedef Int32 Fixed;

typedef struct {
  Fixed x;
  Fixed y;
} vec2;

typedef struct {
  Fixed x;
  Fixed y;
  Fixed z;
} vec3;

typedef struct {
  Int16 i;
  Int16 j;
  Int16 k;
} rot3;

typedef struct {
  vec3 start;
  vec3 end;
} line3d;

typedef struct {
  UInt16 start;
  UInt16 end;
} Edge;

typedef struct {
  UInt16 verts_count;
  UInt16 edges_count;
  vec3* verts;
  Edge* edges;
} Figure;

Figure cube;
UInt32 frameDelay;
RectangleType clearRect = {{0, 10}, {160, 150}};
UInt32 totalRot;

float to_float(Fixed fp) {
  float res;
  Int16 fix_pow = fp & FRAC_MASK;
  Int16 mant = (fp & MANT_MASK) >> FRAC_BITS;
  if (fp & 0x8000) {
    // keep sign if negative
    mant |= SIGN_MASK;
  }

  res = (float) (mant + fix_pow) * (1.0f / (1 << FRAC_BITS));
  return res;
}

vec2 make_vec2(float x, float y) {
  vec2 res = {.x = FLOAT_TO_FIX(x), .y = FLOAT_TO_FIX(y)};
  return res;
}

Edge make_edge(UInt16 start, UInt16 end) {
  Edge res = {.start = start, .end = end};
  return res;
}

vec3 make_vec3(float x, float y, float z) {
  vec3 res = {.x = FLOAT_TO_FIX(x), .y = FLOAT_TO_FIX(y), .z = FLOAT_TO_FIX(z)};
  return res;
}

vec2 point_3d(vec3 point) {
  Fixed x2d, y2d, x_norm, y_norm;
  vec2 res;
  if (point.z < 1) {
    return make_vec2(-1, -1);
  }

  x2d = FIX_DIV(point.x, point.z);
  y2d = FIX_DIV(point.y, point.z);
  x_norm = (x2d + INT_TO_FIX(W3D) / 2) / W3D;
  y_norm = (y2d + INT_TO_FIX(H3D) / 2) / H3D;
  x2d = x_norm * SCREEN_W;
  y2d = (INT_TO_FIX(1) - y_norm) * SCREEN_H;
  res.x = x2d;
  res.y = y2d;
  return res;
}

void draw_line_3d(vec3 start, vec3 end) {
  vec2 start_2d, end_2d;
  if (TO_INT(start.z) <= 0 || TO_INT(end.z) <= 0) {
    // TODO: draw part of the line in view
    return;
  }

  start_2d = point_3d(start);
  end_2d = point_3d(end);
  // TODO: check if line fits screen?

  if ((start_2d.x == -1 && start_2d.y == -1) || (end_2d.x == -1 || end_2d.y == -1)) {
    return;
  }

  WinDrawLine((Coord) TO_INT(start_2d.x), (Coord) TO_INT(start_2d.y), (Coord) TO_INT(end_2d.x),
              (Coord) TO_INT(end_2d.y));
}

void draw_figure(Figure* figure) {
  UInt16 i;
  for (i = 0; i < figure->edges_count; ++i) {
    Edge edge = figure->edges[i];
    draw_line_3d(figure->verts[edge.start], figure->verts[edge.end]);
  }
}

float fsin(Int16 angle) {
  // TODO: negative angle
  return sin_table[angle % 360];
}

// cos(x) = sin(90-x)
float fcos(Int16 angle) {
  int index = (90 - angle) % 360;
  if (index < 0) {
    index = index + 360;
  }
  return sin_table[index];
}

void rotate_3d_fast(vec3* points, UInt16 count, vec3 center, rot3 angle) {
  Fixed cosa, sina, cosb, sinb, cosc, sinc;
  Fixed axx, axy, axz, ayx, ayy, ayz, azx, azy, azz;
  Fixed px, py, pz;
  UInt16 i;

  cosa = FLOAT_TO_FIX(fcos(angle.i));
  sina = FLOAT_TO_FIX(fsin(angle.i));
  cosb = FLOAT_TO_FIX(fcos(angle.j));
  sinb = FLOAT_TO_FIX(fsin(angle.j));
  cosc = FLOAT_TO_FIX(fcos(angle.k));
  sinc = FLOAT_TO_FIX(fsin(angle.k));

  axx = FIX_MUL(cosa, cosb);
  axy = FIX_MUL(FIX_MUL(cosa, sinb), sinc) - FIX_MUL(sina, cosc);
  axz = FIX_MUL(FIX_MUL(cosa, sinb), cosc) + FIX_MUL(sina, sinc);
  ayx = FIX_MUL(sina, cosb);
  ayy = FIX_MUL(FIX_MUL(sina, sinb), sinc) + FIX_MUL(cosa, cosc);
  ayz = FIX_MUL(FIX_MUL(sina, sinb), cosc) - FIX_MUL(cosa, sinc);
  azx = -sinb;
  azy = FIX_MUL(cosb, sinc);
  azz = FIX_MUL(cosb, cosc);

  for (i = 0; i < count; i++) {
    px = points[i].x - center.x;
    py = points[i].y - center.y;
    pz = points[i].z - center.z;

    points[i].x = FIX_MUL(axx, px) + FIX_MUL(axy, py) + FIX_MUL(axz, pz) + center.x;
    points[i].y = FIX_MUL(ayx, px) + FIX_MUL(ayy, py) + FIX_MUL(ayz, pz) + center.y;
    points[i].z = FIX_MUL(azx, px) + FIX_MUL(azy, py) + FIX_MUL(azz, pz) + center.z;
  }
}

void app_handle_event(EventPtr event) {}

void StopGame() {
  MemPtrFree(cube.verts);
  MemPtrFree(cube.edges);
}

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

void InitGame() {
  vec3* verts;
  Edge* edges;

  frameDelay = SysTicksPerSecond() / FPS;

  verts = (vec3*) MemPtrNew(sizeof(vec3) * 8);
  edges = (Edge*) MemPtrNew(sizeof(Edge) * 12);

  cube.verts_count = 8;
  cube.edges_count = 12;
  cube.verts = verts;
  cube.edges = edges;

  verts[0] = make_vec3(-5, -5, 10);
  verts[1] = make_vec3(-5, 5, 10);
  verts[2] = make_vec3(5, 5, 10);
  verts[3] = make_vec3(5, -5, 10);

  verts[4] = make_vec3(-5, -5, 20);
  verts[5] = make_vec3(-5, 5, 20);
  verts[6] = make_vec3(5, 5, 20);
  verts[7] = make_vec3(5, -5, 20);

  edges[0] = make_edge(0, 1);
  edges[1] = make_edge(1, 2);
  edges[2] = make_edge(2, 3);
  edges[3] = make_edge(3, 0);

  edges[4] = make_edge(4, 5);
  edges[5] = make_edge(5, 6);
  edges[6] = make_edge(6, 7);
  edges[7] = make_edge(7, 4);

  edges[8] = make_edge(0, 4);
  edges[9] = make_edge(1, 5);
  edges[10] = make_edge(2, 6);
  edges[11] = make_edge(3, 7);
}

void Update(UInt32 delta) {
  UInt16 rot_speed = 6;
  UInt16 rot_angle = delta * rot_speed / frameDelay;
  rot3 angle = {rot_angle, rot_angle, rot_angle};
  vec3 center = make_vec3(0, 0, 15);
  totalRot += rot_angle;

  rotate_3d_fast(cube.verts, cube.verts_count, center, angle);
  WinEraseRectangle(&clearRect, 0);
  draw_figure(&cube);
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
#ifdef DRAW_FPS
  char fps_buf[255];
  char update_buf[255];
  UInt32 tick_counter = 0;
  UInt16 frame_counter = 0;
  UInt32 TPS = SysTicksPerSecond();
  Int16 fpsStrLen;
#endif

  InitMath();
  InitGame();

  do {
    ts1 = TimGetTicks();
    if (totalRot < 3600) {
      Update(ts1 - ts);
      tsN = TimGetTicks();
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

    EvtGetEvent(&event, evtNoWait);
    if (!SysHandleEvent(&event)) {
      app_handle_event(&event);
    }
  } while (event.eType != appStopEvent);
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
