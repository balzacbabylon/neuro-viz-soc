#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Required for memcpy

int pixel_buffer_start;
#define JTAG_UART_BASE			0xFF201000
	

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
	
	//going to invert the coordinate plane so that x and y start at
	//bottom left of screen

	buffer[239 - y][x] = color;


}
// ToDo: can be re-written in a less messy way for if statement
// ToDo: can this be done without using floating point?

typedef struct {
    int* data;
    size_t length;
} IntArray;

IntArray interpolate(int i0, int d0, int i1, int d1){
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

typedef struct{

	int x;
	int y;

}Point;

void swap(Point* p0, Point* p1){

	int x_t,y_t;
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
		IntArray ys = interpolate(p0.x,p0.y,p1.x,p1.y);
		for(int i = p0.x; i <= p1.x; i++){

			put_pixel(i, ys.data[ i - p0.x], color);

		}

		//free(ys);

	}else{

		//line is more vertical
		//make sure y0 < y1
		if( p0.y > p1.y){
			swap(&p0,&p1);
		}
		IntArray xs = interpolate(p0.y,p0.x,p1.y,p1.x);
		for( int y = p0.y; y <= p1.y; y++){
			put_pixel( xs.data[y - p0.y], y, color);
		}

		//free(xs);

	}


}

IntArray combine_arrays(IntArray a1, IntArray a2) {
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

void drawfilledtriangle(Point P0, Point P1, Point P2, short int color){

	if(P1.y < P0.y) swap(&P1, &P0);
	if(P2.y < P0.y) swap(&P2, &P0);
	if(P2.y < P1.y) swap(&P2, &P1);

	IntArray x01 = interpolate(P0.y, P0.x, P1.y, P1.x);
	IntArray x12 = interpolate(P1.y, P1.x, P2.y, P2.x);
	IntArray x02 = interpolate(P0.y, P0.x, P2.y, P2.x);

	// 1. Calculate length and set up struct
	IntArray x012 = combine_arrays(x01,x12); 

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
	
int main(void){
	
	volatile int * JTAG_UART_ptr = (int *)JTAG_UART_BASE;

    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	
	pixel_buffer_start = *pixel_ctrl_ptr; //reading the VALUE of whats in FF203020
	
	char *str;
	char text_string[50];
	
	sprintf(text_string, "%x", pixel_buffer_start);
	
	for (str = text_string; *str != 0; ++str){
		put_jtag(JTAG_UART_ptr, *str);
	}
	put_jtag(JTAG_UART_ptr, '\n');

	*(pixel_ctrl_ptr + 1) = &buffer;

	Point p0 = {110, 110};
	Point p1 = {170, 150};
	Point p2 = {160, 120};

	drawfilledtriangle(p0, p1, p2, 0xFCCC);
	
	drawline(p0,p1,0xFFFF);
	drawline(p2,p1,0xFFFF);
	drawline(p2,p0,0xFFFF);
	
	/*
	for(int i  = 0; i < 240; i++){
		for(int j = 0; j < 320; j++){
			
			buffer[i][j] = 0xFFFC;
		}
	}*/
	*(pixel_ctrl_ptr) = 1;

}