//
// Created by kirill on 27.12.2020.
//

#ifndef RAILROAD_WIREFRAME_H
#define RAILROAD_WIREFRAME_H

#include <PalmTypes.h>

#include "fixed_point.h"

#define W3D 2
#define H3D 2

#define SCREEN_W 160
#define SCREEN_H 160

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

vec2 make_vec2(float x, float y);

Edge make_edge(UInt16 start, UInt16 end);

vec3 make_vec3(float x, float y, float z);

vec2 point_3d(vec3 point);

void draw_line_3d(vec3 start, vec3 end);

void draw_figure(Figure* figure);

Fixed fsin(Int16 angle);

Fixed fcos(Int16 angle);

void rotate_3d_fast(vec3* points, UInt16 count, vec3 center, rot3 angle);

#endif  // RAILROAD_WIREFRAME_H
