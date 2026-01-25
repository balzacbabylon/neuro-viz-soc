#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"

Point project_vertex(Vertex p);
void drawline(Point p0, Point p1, short int color);
void drawfilledtriangle(Point P0, Point P1, Point P2, short int color);
void drawshadedtriangle(Point P0, Point P1, Point P2, short int color);
void RenderObject(Object obj, PointArray* pj);

#endif