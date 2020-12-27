//
// Created by kirill on 27.12.2020.
#include <PalmOS.h>

//
#include "fixed_point.h"
#include "game.h"
#include "wireframe.h"

#define FPS 30

UInt16 totalRot;
UInt32 frameDelay;
RectangleType clearRect = {{0, 10}, {160, 150}};
Figure cube;

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

Int16 Update(UInt32 delta) {
  UInt16 rot_speed = 3;
  UInt16 rot_angle = delta * rot_speed / frameDelay;
  rot3 angle = {rot_angle, rot_angle, rot_angle};
  vec3 center = make_vec3(0, 0, 15);
  totalRot += rot_angle;

  rotate_3d_fast(cube.verts, cube.verts_count, center, angle);
  WinEraseRectangle(&clearRect, 0);
  draw_figure(&cube);
  if (totalRot < 1000) {
    return 0;
  } else {
    return -1;
  }
}

void StopGame() {
  MemPtrFree(cube.verts);
  MemPtrFree(cube.edges);
}
