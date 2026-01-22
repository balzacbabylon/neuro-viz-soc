#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Required for memcpy
#include <stdint.h>

int pixel_buffer_start;
#define JTAG_UART_BASE			0xFF201000
	
#define CW	320
#define CH	240
#define FOCAL_LENGTH 256

typedef int32_t fixed;

typedef struct { 
	int x; 
	int y; 
} Point2D;

typedef struct {
    int* data;
    size_t length;
} IntArray;

typedef struct {

	float* data;
	size_t length;
} FArray;

typedef struct { 

	fixed data[240];
	int length;

}Scanline;

typedef struct{

	fixed* data;
	size_t length;

}FixedArray;

typedef struct{

	fixed x;
	fixed y;
	fixed z;

}Vertex;


typedef struct{

	int x;
	int y;
	fixed h;

}Point;

// TODO: parameterize the fixed point number (16)
#define FIXED_ONE (1 << 16)
#define INT_TO_FIXED(x) ((x) << 16)
#define FIXED_TO_INT(x) ((x) >> 16)
#define FLOAT_TO_FIXED(f) ((fixed)((f) * 65536.0f))

// Multiply two fixed-point numbers
// Cast to (int64_t) prevents overflow before the division (shift)
static inline fixed mul_fixed(fixed a, fixed b) {
    return (fixed)(((int64_t)a * b) >> 16);
}

// Division
// TODO: There is no overflow/underflow checking...need to add that
static inline fixed div_fixed(fixed a, fixed b) {
    return (fixed)(((int64_t)a << 16) / (int64_t)b);
}

short int buffer[240][512];

void put_jtag(volatile int * JTAG_UART_ptr, char c)
{
	int control;
	control = *(JTAG_UART_ptr + 1); // read the JTAG_UART control register
	if (control & 0xFFFF0000)
	// if space, echo character, else ignore
	*(JTAG_UART_ptr) = c;
}

void put_pixel(int x, int y, short int color){
	
	//setting coordinate system zeros at y = 119 and x = 159
	if (x > 160 || x < -159 || y > 119 || y < -120) return;
	buffer[119 - y][159 + x] = color;

}
// TODO: can be re-written in a less messy way for if statement
// TODO: can this be done without using floating point?


Point project_vertex(Vertex p) {

	Point v;

	v.x = FIXED_TO_INT(div_fixed((mul_fixed(p.x,INT_TO_FIXED(FOCAL_LENGTH))),p.z));
    v.y = FIXED_TO_INT(div_fixed((mul_fixed(p.y,INT_TO_FIXED(FOCAL_LENGTH))),p.z));

	return v;

}


FixedArray interpolate(fixed i0, fixed d0, fixed i1, fixed d1){

	FixedArray res;
	// EDGE CASE 1: Single Point (Avoid Divide by Zero)
	if (i0 == i1){
		res.data = malloc(sizeof(fixed));
		res.data[0] = d0;
		res.length = 1;
		return res;
	}	

	// EDGE CASE 2: Buffer Overflow Protection
    // Ensure we don't write past the end of our static array
	int n = FIXED_TO_INT(i1) - FIXED_TO_INT(i0) + 1;
	fixed dd = d1 - d0;
	fixed di = i1 - i0;

	//TODO: see if di needs to be treated as an int
	fixed step = div_fixed(dd,di);
	fixed d = d0;

	for( int i = 0; i < n ; i++){
		res.data[i] = d;
		d += step;
	}

	return res;

}


IntArray iinterpolate(int i0, int d0, int i1, int d1){
	// assuming i0 < i1 (handled externally then)

	IntArray res;
	if( i0 == i1){
		res.data = malloc(sizeof(int));
		res.data[0] = d0;
		res.length = 1;
		return res;
	}

	int n = i1 - i0 + 1;
	res.length = n;
	res.data = malloc( n * sizeof(int));
	int di = i1 - i0;
	int dd = d1 - d0;
	int step = dd > 0 ? 1 : -1;
	dd = abs(dd);
	int error = -(di/2);
	
	int d = d0;
	for(int i = 0; i < n; i++){
		res.data[i] = d;
		error += dd;
		// Change 'if' to 'while' to allow multiple steps per Y increment
		while(error > 0){
			d += step;
			error -= di;
		}
	}

	return res;

}

FArray finterpolate(int i0, float d0, int i1, float d1){

		FArray res;
		if(i0 == i1){
			res.data = malloc(sizeof(float));
			res.data[0] = d0;
			res.length = 1;
			return res;
		}

		int n = i1 - i0 + 1;
		res.length = n;
		res.data = malloc(n * sizeof(float));
		
		float step = (d1 - d0) / (float)(i1 - i0);
		float d = d0;

		for (int i = 0; i < n; i++)
		{
			res.data[i] = d;
			d += step;
		}
		
		return res;

}

void swap(Point* p0, Point* p1){

	fixed x_t,y_t;
	x_t = p0->x;
	y_t = p0->y;
	p0->x = p1->x;
	p0->y = p1->y;
	p1->x = x_t;
	p1->y = y_t;

}

void drawline(Point p0, Point p1, short int color){

	if( abs(p1.x - p0.x) > abs(p1.y - p0.y)){

		//line is more horizontal
		//make sure x0 < x1
		if( p0.x > p1.x ){
			swap(&p0,&p1);
		}
		IntArray ys = iinterpolate(p0.x,p0.y,p1.x,p1.y);
		for(int i = p0.x; i <= p1.x; i++){

			put_pixel(i, ys.data[ i - p0.x], color);

		}

		free(ys.data);

	}else{

		//line is more vertical
		//make sure y0 < y1
		if( p0.y > p1.y){
			swap(&p0,&p1);
		}
		IntArray xs = iinterpolate(p0.y,p0.x,p1.y,p1.x);
		for( int y = p0.y; y <= p1.y; y++){
			put_pixel( xs.data[y - p0.y], y, color);
		}

		free(xs.data);

	}


}

IntArray combine_iarrays(IntArray a1, IntArray a2) {
    IntArray result;
    
    // 1. Calculate the new length
    // (n-1 from first) + (all n from second)
    result.length = (a1.length - 1) + a2.length;
    
    // 2. Allocate memory
    result.data = malloc(result.length * sizeof(int));
    if (result.data == NULL) return result; // Handle allocation failure

    // 3. Copy (n-1) elements from the first array
    // Syntax: memcpy(destination, source, number_of_bytes)
    memcpy(result.data, a1.data, (a1.length - 1) * sizeof(int));

    // 4. Copy all elements from the second array into the remaining slot
    // We offset the destination pointer by (x01.length - 1)
    memcpy(result.data + (a1.length - 1), a2.data, a2.length * sizeof(int));

    return result;
}

FArray combine_farrays(FArray a1, FArray a2){
	FArray result;

	result.length = (a1.length - 1) + a2.length;
    
    // 2. Allocate memory
    result.data = malloc(result.length * sizeof(float));
    if (result.data == NULL) return result; // Handle allocation failure

    // 3. Copy (n-1) elements from the first array
    // Syntax: memcpy(destination, source, number_of_bytes)
    memcpy(result.data, a1.data, (a1.length - 1) * sizeof(float));

    // 4. Copy all elements from the second array into the remaining slot
    // We offset the destination pointer by (x01.length - 1)
    memcpy(result.data + (a1.length - 1), a2.data, a2.length * sizeof(float));

    return result;
}

void drawfilledtriangle(Point P0, Point P1, Point P2, short int color){

	if(P1.y < P0.y) swap(&P1, &P0);
	if(P2.y < P0.y) swap(&P2, &P0);
	if(P2.y < P1.y) swap(&P2, &P1);

	IntArray x01 = iinterpolate(P0.y, P0.x, P1.y, P1.x);
	IntArray x12 = iinterpolate(P1.y, P1.x, P2.y, P2.x);
	IntArray x02 = iinterpolate(P0.y, P0.x, P2.y, P2.x);

	// 1. Calculate length and set up struct
	IntArray x012 = combine_iarrays(x01,x12); 

	int m = x012.length / 2;
	IntArray x_left, x_right;
	if( x02.data[m] < x012.data[m]){
		x_left = x02;
		x_right = x012;
	}else {
		x_left = x012;
		x_right = x02;
	}

	for( int y = P0.y ; y <= P2.y; y++){

		for( int x = x_left.data[y - P0.y]; x <= x_right.data[y - P0.y] ; x++){
			put_pixel(x,y,color);
		}

	}

	free(x01.data);
	free(x12.data);
	free(x02.data);
	free(x012.data);

}

short int apply_intensity(short int color, float intensity) {
    // mask: 1111 1000 0000 0000
    int r = (color >> 11) & 0x1F; 
    
    // mask: 0000 0111 1110 0000
    int g = (color >> 5) & 0x3F;  
    
    // mask: 0000 0000 0001 1111
    int b = color & 0x1F;         

    // Apply intensity to each channel separately
    r = (int)(r * intensity);
    g = (int)(g * intensity);
    b = (int)(b * intensity);

    // Re-pack the bits
    return (short int)((r << 11) | (g << 5) | b);
}

void drawshadedtriangle( Point P0, Point P1, Point P2, short int color){

	if(P1.y < P0.y) swap(&P1, &P0);
	if(P2.y < P0.y) swap(&P2, &P0);
	if(P2.y < P1.y) swap(&P2, &P1);

	// Compute the x coordinates and h values of the triangle edges
    IntArray x01 = iinterpolate(P0.y, P0.x, P1.y, P1.x);
    FArray h01 = finterpolate(P0.y, P0.h, P1.y, P1.h);

    IntArray x12 = iinterpolate(P1.y, P1.x, P2.y, P2.x);
    FArray h12 = finterpolate(P1.y, P1.h, P2.y, P2.h);

    IntArray x02 = iinterpolate(P0.y, P0.x, P2.y, P2.x);
    FArray h02 = finterpolate(P0.y, P0.h, P2.y, P2.h);

	IntArray x012 = combine_iarrays(x01,x12);
	FArray h012 = combine_farrays(h01,h12);

	int m = x012.length / 2;
	IntArray x_left, x_right;
	FArray h_left, h_right;
	if( x02.data[m] < x012.data[m]){
		x_left = x02;
		h_left = h02;
		x_right = x012;
		h_right = h012;
	}else {
		x_left = x012;
		h_left = h012;
		x_right = x02;
		h_right = h02;
	}

	int x_l, x_r;
	FArray h_segment;
	short int shaded_color;

	for( int y = P0.y ; y <= P2.y; y++){

		x_l = x_left.data[y - P0.y];
		x_r = x_right.data[y - P0.y];

		h_segment = finterpolate(x_l, h_left.data[y - P0.y], x_r, h_right.data[y - P0.y]);

		for( int x = x_l; x <= x_r ; x++){

			shaded_color = apply_intensity(color,h_segment.data[x - x_l]);

			put_pixel(x,y,shaded_color);
		}

		// You must free this, or the FPGA will run out of heap memory immediately
        free(h_segment.data);

	}

	// Cleanup edges
    free(x01.data);  free(h01.data);
    free(x12.data);  free(h12.data);
    free(x02.data);  free(h02.data);
    free(x012.data); free(h012.data);

}
	
int main(void){
	
	volatile int * JTAG_UART_ptr = (int *)JTAG_UART_BASE;

    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	
	pixel_buffer_start = *pixel_ctrl_ptr; //reading the VALUE of whats in FF203020
	
	char *str;
	char text_string[50];
	
	*(pixel_ctrl_ptr + 1) = (int)&buffer;

	/*
	Point p0 = {10, 10,0.0};
	Point p1 = {270, 90,0.5};
	Point p2 = {160, 220,1.0};

	drawfilledtriangle(p0, p1, p2, 0xFCCC);

	drawshadedtriangle(p0,p1,p2, 0xFCCC);
	
	drawline(p0,p1,0xFFFF);
	drawline(p2,p1,0xFFFF);
	drawline(p2,p0,0xFFFF);
	*/

	//TODO: Implement out of bounds checking 
	Vertex vAf = {FLOAT_TO_FIXED(-3.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(5)};
	Vertex vBf = {FLOAT_TO_FIXED(-3.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(5)};
	Vertex vCf = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(5)};
	Vertex vDf = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(5)};

	Vertex vAb = {FLOAT_TO_FIXED(-3.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(8)};
	Vertex vBb = {FLOAT_TO_FIXED(-3.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(8)};
	Vertex vCb = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(8)};
	Vertex vDb = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(8)};


	//back face
	drawline(project_vertex(vAb),project_vertex(vBb),0x001F);
	drawline(project_vertex(vBb),project_vertex(vCb),0x001F);
	drawline(project_vertex(vCb),project_vertex(vDb),0x001F);
	drawline(project_vertex(vDb),project_vertex(vAb),0x001F);

	//front to back edges
	drawline(project_vertex(vAf),project_vertex(vAb),0x07E0);
	drawline(project_vertex(vBf),project_vertex(vBb),0x07E0);
	drawline(project_vertex(vCf),project_vertex(vCb),0x07E0);
	drawline(project_vertex(vDf),project_vertex(vDb),0x07E0);

	// front face
	drawline(project_vertex(vAf),project_vertex(vBf),0xF800);
	drawline(project_vertex(vBf),project_vertex(vCf),0xF800);
	drawline(project_vertex(vCf),project_vertex(vDf),0xF800);
	drawline(project_vertex(vDf),project_vertex(vAf),0xF800);

	*(pixel_ctrl_ptr) = 1;

}