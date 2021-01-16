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

void setPixel(WinHandle handle, int x, int y) {
  UInt8* targetByte = (UInt8*) (handle->displayAddrV20 + ((handle->displayWidthV20 >> 3) * y) + (x >> 3));
  *targetByte = *targetByte | (0x80 >> (x & 7));
}

void plot_line(WinHandle handle, int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    setPixel(handle, x0, y0);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    } /* e_xy+e_x > 0 */
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    } /* e_xy+e_y < 0 */
  }
}

void draw_figure(Figure* figure) {
  WinHandle handle;
  UInt16 i;
  vec2 verts_2d[figure->verts_count];
  for (i = 0; i < figure->verts_count; ++i) {
    verts_2d[i] = point_3d(figure->verts[i]);
  }

  handle = WinGetDrawWindow();

  for (i = 0; i < figure->edges_count; ++i) {
    Edge edge = figure->edges[i];
    if (verts_2d[edge.start].x != -1 && verts_2d[edge.end].x != -1) {
      plot_line(handle, TO_INT(verts_2d[edge.start].x), TO_INT(verts_2d[edge.start].y), TO_INT(verts_2d[edge.end].x),
                TO_INT(verts_2d[edge.end].y));
    }
  }
}

Fixed fsin(Int16 angle) {
  Int16 index = angle % 360;
  if (index < 0) {
    index = index + 360;
  }
  return sin_table[index];
}

// cos(x) = sin(90-x)
Fixed fcos(Int16 angle) {
  Int16 index = (90 - angle) % 360;
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
