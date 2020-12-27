//
// Created by kirill on 27.12.2020.
#include <PalmOS.h>

//
#include "fixed_point.h"
#include "game.h"
#include "wireframe.h"

UInt16 totalRot;
UInt32 frameDelay;
RectangleType clearRect = {{0, 10}, {160, 150}};
Figure cube;
UInt32 rem = 0;

void InitGame() {
  vec3* verts;
  Edge* edges;
  Int16 i = 0;
  frameDelay = SysTicksPerSecond() / FPS;

  verts = (vec3*) MemPtrNew(sizeof(vec3) * 24);
  edges = (Edge*) MemPtrNew(sizeof(Edge) * 48);

  cube.verts_count = 24;
  cube.edges_count = 48;
  cube.verts = verts;
  cube.edges = edges;

  verts[i++] = make_vec3(10 - 15, 10 - 15, 00 + 15);
  verts[i++] = make_vec3(10 - 15, 20 - 15, 00 + 15);
  verts[i++] = make_vec3(20 - 15, 10 - 15, 00 + 15);
  verts[i++] = make_vec3(20 - 15, 20 - 15, 00 + 15);
  verts[i++] = make_vec3(10 - 15, 00 - 15, 10 + 15);
  verts[i++] = make_vec3(20 - 15, 00 - 15, 10 + 15);
  verts[i++] = make_vec3(00 - 15, 10 - 15, 10 + 15);
  verts[i++] = make_vec3(00 - 15, 20 - 15, 10 + 15);
  verts[i++] = make_vec3(10 - 15, 30 - 15, 10 + 15);
  verts[i++] = make_vec3(20 - 15, 30 - 15, 10 + 15);
  verts[i++] = make_vec3(30 - 15, 10 - 15, 10 + 15);
  verts[i++] = make_vec3(30 - 15, 20 - 15, 10 + 15);
  verts[i++] = make_vec3(10 - 15, 00 - 15, 20 + 15);
  verts[i++] = make_vec3(20 - 15, 00 - 15, 20 + 15);
  verts[i++] = make_vec3(00 - 15, 10 - 15, 20 + 15);
  verts[i++] = make_vec3(00 - 15, 20 - 15, 20 + 15);
  verts[i++] = make_vec3(10 - 15, 30 - 15, 20 + 15);
  verts[i++] = make_vec3(20 - 15, 30 - 15, 20 + 15);
  verts[i++] = make_vec3(30 - 15, 10 - 15, 20 + 15);
  verts[i++] = make_vec3(30 - 15, 20 - 15, 20 + 15);
  verts[i++] = make_vec3(10 - 15, 10 - 15, 30 + 15);
  verts[i++] = make_vec3(10 - 15, 20 - 15, 30 + 15);
  verts[i++] = make_vec3(20 - 15, 10 - 15, 30 + 15);
  verts[i++] = make_vec3(20 - 15, 20 - 15, 30 + 15);

  i = 0;

  edges[i++] = make_edge(0, 1);
  edges[i++] = make_edge(1, 3);
  edges[i++] = make_edge(3, 2);
  edges[i++] = make_edge(2, 0);

  edges[i++] = make_edge(4, 5);
  edges[i++] = make_edge(5, 10);
  edges[i++] = make_edge(10, 11);
  edges[i++] = make_edge(11, 9);
  edges[i++] = make_edge(9, 8);
  edges[i++] = make_edge(8, 7);
  edges[i++] = make_edge(7, 6);
  edges[i++] = make_edge(6, 4);

  edges[i++] = make_edge(12, 13);
  edges[i++] = make_edge(13, 18);
  edges[i++] = make_edge(18, 19);
  edges[i++] = make_edge(19, 17);
  edges[i++] = make_edge(17, 16);
  edges[i++] = make_edge(16, 15);
  edges[i++] = make_edge(15, 14);
  edges[i++] = make_edge(14, 12);

  edges[i++] = make_edge(20, 21);
  edges[i++] = make_edge(21, 23);
  edges[i++] = make_edge(23, 22);
  edges[i++] = make_edge(22, 20);

  edges[i++] = make_edge(0, 4);
  edges[i++] = make_edge(4, 12);
  edges[i++] = make_edge(12, 20);

  edges[i++] = make_edge(0, 6);
  edges[i++] = make_edge(6, 14);
  edges[i++] = make_edge(14, 20);

  edges[i++] = make_edge(1, 7);
  edges[i++] = make_edge(7, 15);
  edges[i++] = make_edge(15, 21);

  edges[i++] = make_edge(1, 8);
  edges[i++] = make_edge(8, 16);
  edges[i++] = make_edge(16, 21);

  edges[i++] = make_edge(2, 5);
  edges[i++] = make_edge(5, 13);
  edges[i++] = make_edge(13, 22);

  edges[i++] = make_edge(2, 10);
  edges[i++] = make_edge(10, 18);
  edges[i++] = make_edge(18, 22);

  edges[i++] = make_edge(3, 9);
  edges[i++] = make_edge(9, 17);
  edges[i++] = make_edge(17, 23);

  edges[i++] = make_edge(3, 11);
  edges[i++] = make_edge(11, 19);
  edges[i++] = make_edge(19, 23);
}

Int16 Update(UInt32 delta) {
  UInt16 rot_speed = 1;
  UInt16 rot_angle = (delta + rem) * rot_speed / frameDelay;
  rot3 angle = {rot_angle, rot_angle, rot_angle};
  vec3 center = make_vec3(0, 0, 30);
  rem = (delta + rem) * rot_speed % frameDelay;
  totalRot += rot_angle;

  rotate_3d_fast(cube.verts, cube.verts_count, center, angle);
  WinEraseRectangle(&clearRect, 0);
  draw_figure(&cube);
  if (totalRot < 360) {
    return 0;
  } else {
    return -1;
  }
}

void StopGame() {
  MemPtrFree(cube.verts);
  MemPtrFree(cube.edges);
}
