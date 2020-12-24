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

typedef struct {
  float x;
  float y;
} vec2;

typedef struct {
  float x;
  float y;
  float z;
} vec3;

typedef struct {
  float i;
  float j;
  float k;
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

vec2 make_vec2(float x, float y) {
  vec2 res = {.x = x, .y = y};
  return res;
}

Edge make_edge(UInt16 start, UInt16 end) {
  Edge res = {.start = start, .end = end};
  return res;
}

vec3 make_vec3(float x, float y, float z) {
  vec3 res = {.x = x, .y = y, .z = z};
  return res;
}

vec2 point_3d(vec3 point) {
  float x2d, y2d, x_norm, y_norm;
  if (point.z < 1) {
    return make_vec2(-1, -1);
  }

  x2d = point.x / point.z;
  y2d = point.y / point.z;
  x_norm = (x2d + W3D / 2) / W3D;
  y_norm = (y2d + H3D / 2) / H3D;
  x2d = x_norm * SCREEN_W;
  y2d = (1 - y_norm) * SCREEN_H;
  return make_vec2(x2d, y2d);
}

void draw_line_3d(vec3 start, vec3 end) {
  vec2 start_2d, end_2d;
  if (start.z <= 0.001 || end.z <= 0.001) {
    // TODO: draw part of the line in view
    WinDrawLine(0, 0, 160, 160);
    return;
  }

  start_2d = point_3d(start);
  end_2d = point_3d(end);
  // TODO: check if line fits screen?

  if ((start_2d.x == -1 && start_2d.y == -1) || (end_2d.x == -1 || end_2d.y == -1)) {
    WinDrawLine(0, 160, 160, 0);
    return;
  }

  WinDrawLine((Coord) start_2d.x, (Coord) start_2d.y, (Coord) end_2d.x, (Coord) end_2d.y);
}

void draw_lines_3d(line3d* lines, UInt16 size) {
  UInt16 i = 0;
  for (i; i < size; i++) {
    draw_line_3d(lines[i].start, lines[i].end);
  }
}

void draw_figure(Figure* figure) {
  UInt16 i;
  for (i = 0; i < figure->edges_count; ++i) {
    Edge edge = figure->edges[i];
    draw_line_3d(figure->verts[edge.start], figure->verts[edge.end]);
  }
}

double v2dist(vec2 v1, vec2 v2) {
  return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

vec2 rot_2d(float x, float y, float cx, float cy, float angle) {
  vec2 res;
  float da, dist;
  dist = (float) v2dist(make_vec2(x, y), make_vec2(cx, cy));
  da = (float) atan2(y - cy, x - cx);
  res.x = dist * cos(angle + da) + cx;
  res.y = dist * sin(angle + da) + cy;
  return res;
}

void rotate_3d(vec3* points, UInt16 count, vec3 center, rot3 angle) {
  UInt16 i;
  vec2 rx, ry, rz;
  for (i = 0; i < count; i++) {
    rx = rot_2d(points[i].x, points[i].y, center.x, center.y, angle.i);
    points[i].x = rx.x;
    points[i].y = rx.y;
    ry = rot_2d(points[i].x, points[i].z, center.x, center.z, angle.j);
    points[i].x = ry.x;
    points[i].z = ry.y;
    rz = rot_2d(points[i].y, points[i].z, center.y, center.z, angle.k);
    points[i].y = rz.x;
    points[i].z = rz.y;
  }
}

float fsin(int angle) {
  return sin_table[angle % 360];
}

// cos(x) = sin(90-x)
float fcos(int angle) {
  int index = (90 - angle) % 360;
  if (index < 0) {
    index = index + 360;
  }
  return sin_table[index];
}

void app_handle_event(EventPtr event) {}

void StopApplication() {
  Err error;
  UInt usecount;

  error = MathLibClose(MathLibRef, &usecount);
  ErrFatalDisplayIf(error, "Can't close MathLib");
  if (usecount == 0) SysLibRemove(MathLibRef);
}

void StartApplication() {
  EventType event;
  UInt32 delay = SysTicksPerSecond() / 30;
  UInt16 i = 0;
  Figure cube;
  Err error;
  vec3* verts;
  Edge* edges;

  char buf[255];
  Int16 len;

  rot3 angle = {0.1, 0.1, 0.1};
  vec3 center = {0, 0, 15};

  // Init math lib
  error = SysLibFind(MathLibName, &MathLibRef);
  if (error) error = SysLibLoad(LibType, MathLibCreator, &MathLibRef);
  ErrFatalDisplayIf(error, "Can't find MathLib");  // Just an example; handle it gracefully
  error = MathLibOpen(MathLibRef, MathLibVersion);
  ErrFatalDisplayIf(error, "Can't open MathLib");

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

  len = StrPrintF(buf, "%d", (int) v2dist(make_vec2(0, 0), make_vec2(3, 4)));

  do {
    rotate_3d(cube.verts, cube.verts_count, center, angle);
    WinEraseWindow();
//    WinDrawChars(buf, len, 0, 0);
    draw_figure(&cube);
    EvtGetEvent(&event, delay);
    if (!SysHandleEvent(&event)) {
      app_handle_event(&event);
    }
  } while (event.eType != appStopEvent);

  MemPtrFree(verts);
  MemPtrFree(edges);
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
