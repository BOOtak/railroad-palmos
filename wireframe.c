//
// Created by kirill on 27.12.2020.
//

#include "wireframe.h"

#include <PalmOS.h>

#include "tables.h"

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
  if (start.z <= 0 || end.z <= 0) {
    // TODO: draw part of the line in view
    return;
  }

  start_2d = point_3d(start);
  end_2d = point_3d(end);
  // TODO: check if line fits screen?

  if (start_2d.x == -1 || start_2d.y == -1 || end_2d.x == -1 || end_2d.y == -1) {
    return;
  }

  WinDrawLine((Coord) TO_INT(start_2d.x), (Coord) TO_INT(start_2d.y), (Coord) TO_INT(end_2d.x),
              (Coord) TO_INT(end_2d.y));
}

void draw_figure(Figure* figure) {
  UInt16 i;
  vec2 verts_2d[figure->verts_count];
  for (i = 0; i < figure->verts_count; ++i) {
    verts_2d[i] = point_3d(figure->verts[i]);
  }

  for (i = 0; i < figure->edges_count; ++i) {
    Edge edge = figure->edges[i];
    if (verts_2d[edge.start].x != -1 && verts_2d[edge.end].x != -1) {
      WinDrawLine(TO_INT(verts_2d[edge.start].x), TO_INT(verts_2d[edge.start].y), TO_INT(verts_2d[edge.end].x),
                  TO_INT(verts_2d[edge.end].y));
    }
  }
}

Fixed fsin(Int16 angle) {
  return sin_table[angle % 360];
}

// cos(x) = sin(90-x)
Fixed fcos(Int16 angle) {
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

  cosa = fcos(angle.i);
  sina = fsin(angle.i);
  cosb = fcos(angle.j);
  sinb = fsin(angle.j);
  cosc = fcos(angle.k);
  sinc = fsin(angle.k);

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
