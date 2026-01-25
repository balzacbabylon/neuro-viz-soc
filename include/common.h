#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

// Constants
#define CW  320
#define CH  240
#define FOCAL_LENGTH 256

// Colors RGB565
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define PURPLE  0x8010
#define CYAN    0x07FF
#define BLACK   0x0000
#define WHITE   0xFFFF
#define GRAY    0x8410

// Typedefs
typedef int32_t fixed;

typedef struct { 
    int x; 
    int y; 
} Point2D;

typedef struct {
    int x;
    int y;
    fixed h;
} Point;

typedef struct{

    Point* data;
    size_t length;

}PointArray;

typedef struct Vertex {

    fixed x;
    fixed y;
    fixed z;
    fixed nx;
    fixed ny;
    fixed nz;
    short int color;
    //fixed h;

} Vertex;

typedef struct VertexArray{

    Vertex* data;
    size_t length;

}VertexArray;

typedef struct{

    int i0;
    int i1;
    int i2;
    //short int color;

}Triangle;

typedef struct{
    
    Triangle* data;
    size_t length;

}TriangleArray;

typedef struct{

    VertexArray v; 
    TriangleArray t;
    short int* tca;
    
}Object;

// Array wrappers (Consider renaming to DynamicIntArray etc.)
typedef struct {
    int* data;
    size_t length;
} IntArray;

typedef struct {
    float* data;
    size_t length;
} FArray;

typedef struct {
    fixed* data;
    size_t length;
} FixedArray;

#endif //COMMON_H