#include "common.h"
#include "fixed_math.h"
#include "renderer.h"
#include "platform.h"
#include "lh_inflated.h"
//#include "cube.h"
#include <stdlib.h>
	
int main(void){

	platform_init();

	/*
	* NOTE: draw functions won't work with point coordinates
	* they need to be scaled properly
	Point p0 = {10, 10,0.0};
	Point p1 = {270, 90,0.5};
	Point p2 = {160, 220,1.0};

	drawfilledtriangle(p0, p1, p2, 0xFCCC);

	drawshadedtriangle(p0,p1,p2, 0xFCCC);
	
	drawline(p0,p1,0xFFFF);
	drawline(p2,p1,0xFFFF);
	drawline(p2,p0,0xFFFF);
	*/

	/*
	//TODO: Implement out of bounds checking 
	Vertex vAf = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(5)};
	Vertex vBf = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(5)};
	Vertex vCf = {FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(5)};
	Vertex vDf = {FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(5)};

	Vertex vAb = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(8)};
	Vertex vBb = {FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(8)};
	Vertex vCb = {FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(8)};
	Vertex vDb = {FLOAT_TO_FIXED(1.0),FLOAT_TO_FIXED(-1.0),FLOAT_TO_FIXED(8)};

	
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
	*/
	
	
	Object o;
	o.v.data = malloc(_NUM_VERTS*sizeof(Vertex));
	for(int i = 0; i < _NUM_VERTS; i++){
		o.v.data[i] = _vertices[i];
	}
	o.v.length = _NUM_VERTS;
	o.t.data = malloc(_NUM_TRIANGLES*sizeof(Triangle));
	for(int i = 0; i < _NUM_TRIANGLES; i++){
		o.t.data[i] = _indices[i];
	}
	o.t.length = _NUM_TRIANGLES;
	o.tca = malloc(_NUM_TRIANGLES*sizeof(short int));
	for(int i = 0; i < _NUM_TRIANGLES; i++){
		o.tca[i] = _colorarray[i];
	}

	printuart("created object successfully");
	//Point pjdat[_NUM_VERTS];
	PointArray projected;
	projected.data = malloc(_NUM_VERTS*sizeof(Point));
	projected.length = _NUM_VERTS;

	fixed d = CalculateAutoPosition(&o, 1);
	printuart("calculated distance");
	
	//RenderObject(o, &projected,60.0,d);
	

	//platform_swap_buffers();

	// --- NEW POLLING LOOP ---
    int angle_y = 0; // Previously "current_angle"
    int angle_x = 0; // New X-axis angle

	while(1) {
        platform_clear_screen();

        int keys = platform_read_keys();

        // --- Y AXIS CONTROLS (Yaw) ---
        if (keys & 0x1) angle_y += 5; // KEY 0
        if (keys & 0x8) angle_y -= 5; // KEY 3

        // --- X AXIS CONTROLS (Pitch) ---
        // Bit 1 is KEY1, Bit 2 is KEY2
        if (keys & 0x2) angle_x += 5; // KEY 1
        if (keys & 0x4) angle_x -= 5; // KEY 2

        // Normalize Angles (0-360)
        if (angle_y >= 360) angle_y -= 360;
        if (angle_y < 0)    angle_y += 360;

        if (angle_x >= 360) angle_x -= 360;
        if (angle_x < 0)    angle_x += 360;

        // Pass both angles to renderer
        RenderObject(o, &projected, angle_x, angle_y, d);
		//printuart("rendered object");

        platform_swap_buffers();
    }

}