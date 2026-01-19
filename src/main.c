#include <stdio.h>
#include <stdlib.h>

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
int* interpolate(int i0, int d0, int i1, int d1){
	// assuming i0 < i1 (handled externally then)
	if( i0 == i1){
		int* arr = malloc(sizeof(int));
		arr[0] = d0;
		return arr;
	}
	int n = i1 - i0 + 1;
	int* arr = malloc( n * sizeof(int));
	int di = i1 - i0;
	int dd = d1 - d0;
	int step = dd > 0 ? 1 : -1;
	dd = abs(dd);
	int error = -(di/2);
	
	int d = d0;
	for(int i = 0; i < n; i++){
		arr[i] = d;
		error += dd;
		if( error > 0){
			d += step;
			error -= di;
		}
	}

	return arr;

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
		int* ys = interpolate(p0.x,p0.y,p1.x,p1.y);
		for(int i = p0.x; i <= p1.x; i++){

			put_pixel(i, ys[ i - p0.x], color);

		}

		free(ys);

	}else{

		//line is more vertical
		//make sure y0 < y1
		if( p0.y > p1.y){
			swap(&p0,&p1);
		}
		int* xs = interpolate(p0.y,p0.x,p1.y,p1.x);
		for( int y = p0.y; y <= p1.y; y++){
			put_pixel( xs[y - p0.y], y, color);
		}

		free(xs);

	}


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