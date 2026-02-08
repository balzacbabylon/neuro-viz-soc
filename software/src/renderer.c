#include "renderer.h"
#include "fixed_math.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <math.h> // for abs

// --- Internal Helper Functions (Marked static) ---

static void swap(Point* p0, Point* p1) {
    
	Point temp = *p0;
    *p0 = *p1;
    *p1 = temp;

}


static FixedArray interpolate(fixed i0, fixed d0, fixed i1, fixed d1){

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

static IntArray iinterpolate(int i0, int d0, int i1, int d1) {

    // TODO: optimize regarding malloc
    // assuming i0 < i1 (handled externally then)

	IntArray res;
	if( i0 == i1){
		res.data = malloc(sizeof(int));
		res.data[0] = d0;
		res.length = 1;
		return res;
	}

	int n = i1 - i0 + 1;

	// Safety Check: Limit Size to prevent heap exhaustion
    if (n > 640 || n <= 0) {
        res.data = NULL;
        res.length = 0;
        return res;
    }

	res.length = n;
	res.data = malloc( n * sizeof(int));
    
    // Safety Check: Allocation failure
    if (res.data == NULL) {
        res.length = 0;
        return res;
    }
    
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

static FArray finterpolate(int i0, float d0, int i1, float d1) {
    FArray res;
    if(i0 == i1){
        res.data = malloc(sizeof(float));
        res.data[0] = d0;
        res.length = 1;
        return res;
    }

    int n = i1 - i0 + 1;

    // Safety Check: Limit Size to prevent heap exhaustion
    if (n > 640 || n <= 0) {
        res.data = NULL;
        res.length = 0;
        return res;
    }

    res.length = n;
    res.data = malloc(n * sizeof(float));
    
    // Safety Check: Allocation failure
    if (res.data == NULL) {
        res.length = 0;
        return res;
    }
    
    float step = (d1 - d0) / (float)(i1 - i0);
    float d = d0;

    for (int i = 0; i < n; i++)
    {
        res.data[i] = d;
        d += step;
    }
    
    return res;
}

static IntArray combine_iarrays(IntArray a1, IntArray a2) {
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


static FArray combine_farrays(FArray a1, FArray a2){
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

static short int apply_intensity(short int color, float intensity) {
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

// --- CLIPPING HELPERS ---

#define NEAR_PLANE FIXED_TO_INT(1) // Z = 1.0

// Linearly interpolate a Vertex between a and b based on t (0.0 to 1.0)
static Vertex interpolate_vertex(Vertex a, Vertex b, fixed t) {
    Vertex r;
    r.x = a.x + mul_fixed(t, b.x - a.x);
    r.y = a.y + mul_fixed(t, b.y - a.y);
    r.z = a.z + mul_fixed(t, b.z - a.z); // Should be plane_z effectively
    
    // Interpolate Normals
    r.nx = a.nx + mul_fixed(t, b.nx - a.nx);
    r.ny = a.ny + mul_fixed(t, b.ny - a.ny);
    r.nz = a.nz + mul_fixed(t, b.nz - a.nz);
    
    // Interpolate Color (Simple Gouraud-ish)
    // We unpack high/low byte approx or do channel wise
    // For speed/simplicity here, we just take B's color or A's color? 
    // Ideally we interpolate RGB channels.
    // Let's assume flat shading for color index or simple blend.
    r.color = b.color; 
    return r;
}

// Intersect a line segment (a, b) with the Near Plane (Z = 1.0)
// Returns the intersection vertex.
static Vertex intersect_plane(Vertex a, Vertex b) {
    // Plane Equation: Z = NEAR_PLANE
    // Ray: P = A + t(B - A)
    // P.z = A.z + t(B.z - A.z) = NEAR_PLANE
    // t = (NEAR_PLANE - A.z) / (B.z - A.z)
    
    fixed plane_z = INT_TO_FIXED(1); // 1.0 fixed
    fixed num = plane_z - a.z;
    fixed den = b.z - a.z;
    
    // Avoid divide by zero if line is parallel (shouldn't happen if clipped correctly)
    if (den == 0) return a; 

    fixed t = div_fixed(num, den);
    return interpolate_vertex(a, b, t);
}

// Clips a triangle against the Near Plane Z=1.0
// Outputs 0, 1, or 2 triangles into out_tris array.
// Returns number of triangles produced.
static int ClipTriangle(Vertex v0, Vertex v1, Vertex v2, Vertex out_tris[6]) {
    // 1. Classify vertices (Inside = Z > NearPlane)
    // Using simple integer compare since Fixed Point is just scaled int
    fixed plane = INT_TO_FIXED(1);
    
    int in0 = (v0.z > plane);
    int in1 = (v1.z > plane);
    int in2 = (v2.z > plane);
    
    int in_count = in0 + in1 + in2;
    
    if (in_count == 0) {
        // All outside - Discard
        return 0;
    } else if (in_count == 3) {
        // All inside - Pass through
        out_tris[0] = v0;
        out_tris[1] = v1;
        out_tris[2] = v2;
        return 1;
    } else if (in_count == 1) {
        // 1 Inside, 2 Outside -> Clip to 1 Triangle
        // We keep the inside vertex, and form 2 new ones at intersection
        
        // Align so v0 is the INSIDE vertex
        if (in1) { Vertex t=v0; v0=v1; v1=v2; v2=t; }
        else if (in2) { Vertex t=v0; v0=v2; v2=v1; v1=t; }
        
        // Now v0 is inside, v1/v2 outside
        // New Triangle: v0, intersect(v0,v1), intersect(v0,v2)
        out_tris[0] = v0;
        out_tris[1] = intersect_plane(v0, v1);
        out_tris[2] = intersect_plane(v0, v2);
        return 1;
        
    } else if (in_count == 2) {
        // 2 Inside, 1 Outside -> Clip to 2 Triangles (Quad)
        
        // Align so v2 is the OUTSIDE vertex
        if (!in0) { Vertex t=v2; v2=v0; v0=v1; v1=t; }
        else if (!in1) { Vertex t=v2; v2=v1; v1=v0; v0=t; }
        
        // Now v0/v1 inside, v2 outside.
        // Quad formed by: v0, v1, int(v1,v2), int(v0,v2)
        // Split into 2 tris:
        // T1: v0, v1, int(v1,v2)
        // T2: v0, int(v1,v2), int(v0,v2)
        
        Vertex i1 = intersect_plane(v1, v2);
        Vertex i2 = intersect_plane(v0, v2);
        
        // T1
        out_tris[0] = v0;
        out_tris[1] = v1;
        out_tris[2] = i1;
        
        // T2
        out_tris[3] = v0;
        out_tris[4] = i1;
        out_tris[5] = i2;
        
        return 2;
    }
    
    return 0; // Should not reach
}



// --- VIEWPORT CLIPPING (Cohen-Sutherland) ---

// Screen Boundaries (Centered at 0,0)
#define X_MIN -160
#define X_MAX 159
#define Y_MIN -119
#define Y_MAX 120

// Region Codes
#define INSIDE 0 // 0000
#define LEFT   1 // 0001
#define RIGHT  2 // 0010
#define BOTTOM 4 // 0100
#define TOP    8 // 1000

// Compute the region code for a point (x, y)
static int ComputeOutCode(int x, int y) {
    int code = INSIDE;

    if (x < X_MIN)       // to the left of clip window
        code |= LEFT;
    else if (x > X_MAX)  // to the right of clip window
        code |= RIGHT;
    
    if (y < Y_MIN)       // below the clip window
        code |= BOTTOM;
    else if (y > Y_MAX)  // above the clip window
        code |= TOP;

    return code;
}

// Cohen-Sutherland clipping algorithm clips a line from
// P0 to P1 against a rectangle with diagonal from (X_MIN, Y_MIN) to (X_MAX, Y_MAX).
// Returns 1 if the line is (partially) visible, 0 if rejected.
static int CohenSutherlandClip(Point* p0, Point* p1) {
    int code0 = ComputeOutCode(p0->x, p0->y);
    int code1 = ComputeOutCode(p1->x, p1->y);
    int accept = 0;

    while (1) {
        if ((code0 | code1) == 0) {
            // Bitwise OR is 0: both points inside window; trivially accept and exit loop
            accept = 1;
            break;
        } else if (code0 & code1) {
            // Bitwise AND is not 0: both points share an outside zone (e.g. both left);
            // trivially reject and exit loop
            break;
        } else {
            // Failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            int x = 0, y = 0;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcode = code0 ? code0 : code1;

            // Now find the intersection point;
            // use formulas:
            //   slope = (y1 - y0) / (x1 - x0)
            //   x = x0 + (1/slope) * (ym - y0), where ym is ymin or ymax
            //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
            
            // NOTE: We use floating point for intersection calculation precision, 
            // then cast back to int. This is acceptable for 2D rasterization.
            // Alternatively we could use fixed point if speed is critical, 
            // but these calculations are per-line, not per-pixel.
            
            if (outcode & TOP) {           // point is above the clip window
                x = p0->x + (p1->x - p0->x) * (Y_MAX - p0->y) / (double)(p1->y - p0->y);
                y = Y_MAX;
            } else if (outcode & BOTTOM) { // point is below the clip window
                x = p0->x + (p1->x - p0->x) * (Y_MIN - p0->y) / (double)(p1->y - p0->y);
                y = Y_MIN;
            } else if (outcode & RIGHT) {  // point is to the right of clip window
                y = p0->y + (p1->y - p0->y) * (X_MAX - p0->x) / (double)(p1->x - p0->x);
                x = X_MAX;
            } else if (outcode & LEFT) {   // point is to the left of clip window
                y = p0->y + (p1->y - p0->y) * (X_MIN - p0->x) / (double)(p1->x - p0->x);
                x = X_MIN;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcode == code0) {
                p0->x = x;
                p0->y = y;
                code0 = ComputeOutCode(p0->x, p0->y);
            } else {
                p1->x = x;
                p1->y = y;
                code1 = ComputeOutCode(p1->x, p1->y);
            }
        }
    }
    return accept;
}

// --- Public Interface ---

Point project_vertex(Vertex p) {
    Point v;
    
    // Safety check for Z=0 (should be handled by clipping, but good to have)
    if (p.z == 0) { 
        v.x = 0; 
        v.y = 0; 
        return v; 
    }

    // Use 64-bit math to prevent overflow of (x * focal) during projection.
    // p.x is fixed (int32), FOCAL_LENGTH is int (256).
    // The result represents pixel coordinates directly.
    int64_t x_num = (int64_t)p.x * (int64_t)FOCAL_LENGTH;
    int64_t y_num = (int64_t)p.y * (int64_t)FOCAL_LENGTH;

    v.x = (int)(x_num / p.z);
    v.y = (int)(y_num / p.z);
    return v;
}

void drawline(Point p0, Point p1, short int color) {

    // CLIP: 2D Viewport Clipping
    // This modifies p0 and p1 in-place to fit within the screen.
    // If the line is fully off-screen, it returns 0.
    if (!CohenSutherlandClip(&p0, &p1)) {
        return; 
    }

    if( abs(p1.x - p0.x) > abs(p1.y - p0.y)){

    //line is more horizontal
    //make sure x0 < x1
    if( p0.x > p1.x ){
        swap(&p0,&p1);
    }
    IntArray ys = iinterpolate(p0.x,p0.y,p1.x,p1.y);
    
    // Safety Check: Memory allocation failed or line too long
    if (ys.data == NULL) return;

    for(int i = p0.x; i <= p1.x; i++){
        // Calculate Y index safely
        int idx = i - p0.x;
        if (idx < 0 || idx >= ys.length) continue;
        
        int y = ys.data[idx];
        
        // Final Bounds Check: Ensure pixel is on screen
        if (i >= X_MIN && i <= X_MAX && y >= Y_MIN && y <= Y_MAX) {
             put_pixel(i, y, color);
        }
    }

    free(ys.data);

	}else{

		//line is more vertical
		//make sure y0 < y1
		if( p0.y > p1.y){
			swap(&p0,&p1);
		}
		IntArray xs = iinterpolate(p0.y,p0.x,p1.y,p1.x);
        
        // Safety Check: Memory allocation failed or line too long
        if (xs.data == NULL) return;

		for( int y = p0.y; y <= p1.y; y++){
            // Calculate X index safely
            int idx = y - p0.y;
            if (idx < 0 || idx >= xs.length) continue;
            
            int x = xs.data[idx];
            
            // Final Bounds Check
            if (x >= X_MIN && x <= X_MAX && y >= Y_MIN && y <= Y_MAX) {
			    put_pixel(x, y, color);
            }
		}

		free(xs.data);

	}

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

void drawshadedtriangle(Point P0, Point P1, Point P2, short int color) {

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

static void DrawWireFrameTriangle(Point P0, Point P1, Point P2, short int color){

	//TODO implement method using color vertex
	drawline(P0,P1,color);
	drawline(P2,P1,color);
	drawline(P2,P0,color);

}

// Unused wrapper
// static void RenderTriangle(Triangle t, PointArray* pj, short int color){
// 	DrawWireFrameTriangle(pj->data[t.i0],pj->data[t.i1],pj->data[t.i2],color);
// }


// Simple rotation around Y axis (spinning)
// You can add X-axis rotation similarly
// Simple rotation around Y, X, and Z axes
Vertex apply_transform(Vertex v, int angle_x, int angle_y, int angle_z, fixed trans_x, fixed trans_y, fixed trans_z) {
    Vertex r;
    
    // --- 1. ROTATION Y (Yaw) ---
    // Rotates around the vertical axis (Left/Right spin)
    // x' = x*cos(y) - z*sin(y)
    // z' = x*sin(y) + z*cos(y)
    fixed cos_y = fixed_cos(angle_y);
    fixed sin_y = fixed_sin(angle_y);

    fixed yaw_x = mul_fixed(v.x, cos_y) - mul_fixed(v.z, sin_y);
    fixed yaw_y = v.y;
    fixed yaw_z = mul_fixed(v.x, sin_y) + mul_fixed(v.z, cos_y);

    // --- 2. ROTATION X (Pitch) ---
    // Rotates around the horizontal axis (Up/Down spin)
    // y' = y*cos(x) - z*sin(x)
    // z' = y*sin(x) + z*cos(x)
    fixed cos_x = fixed_cos(angle_x);
    fixed sin_x = fixed_sin(angle_x);

    fixed pitch_x = yaw_x;
    fixed pitch_y = mul_fixed(yaw_y, cos_x) - mul_fixed(yaw_z, sin_x);
    fixed pitch_z = mul_fixed(yaw_y, sin_x) + mul_fixed(yaw_z, cos_x);

    // --- 3. ROTATION Z (Roll) - NEW ---
    // Rotates around the forward axis (Clockwise/Counter-Clockwise tilt)
    // x' = x*cos(z) - y*sin(z)
    // y' = x*sin(z) + y*cos(z)
    fixed cos_z = fixed_cos(angle_z);
    fixed sin_z = fixed_sin(angle_z);

    r.x = mul_fixed(pitch_x, cos_z) - mul_fixed(pitch_y, sin_z);
    r.y = mul_fixed(pitch_x, sin_z) + mul_fixed(pitch_y, cos_z);
    r.z = pitch_z;

    // --- 4. TRANSLATION ---
    r.x = r.x + trans_x;
    r.y = r.y + trans_y;
    r.z = r.z + trans_z; 

    r.color = v.color;
    return r;
}


// Note: We modified the pipeline.
// 1. Transform all vertices to World View Space (apply_transform)
// 2. We CANNOT simply project all vertices first because Z <= 0 will crash.
// 3. Instead, we transform vertices, then loop through TRIANGLES.
// 4. For each triangle, we fetch the 3 Transformed Vertices.
// 5. We CLIP the triangle (producing 0, 1, or 2 triangles).
// 6. We PROJECT the resulting vertices.
// 7. We Draw.

void RenderObject(Object obj, PointArray* pj, int angle_x, int angle_y, int angle_z, fixed offset_z){

    // 1. Buffer for Transformed Vertices (View Space)
    // We must malloc this since we don't want to modify the source Object
    Vertex* view_verts = malloc(obj.v.length * sizeof(Vertex));
    if (!view_verts) return; // safety

    // Transform ALL vertices to View Space first (optimization: only once per frame)
    for(int i = 0; i < obj.v.length; i++ ){
        view_verts[i] = apply_transform(obj.v.data[i], angle_x, angle_y, angle_z, 0, 0, offset_z);
    }
    
    // 2. Loop through Indices (Triangles)
    Vertex out_tris[6]; // Buffer for clipped result (max 2 triangles * 3 verts)
    
    for(int j = 0; j < obj.t.length; j++){
        Triangle t = obj.t.data[j];
        
        // Fetch view-space vertices
        Vertex v0 = view_verts[t.i0];
        Vertex v1 = view_verts[t.i1];
        Vertex v2 = view_verts[t.i2];
        
        // CLIP
        int tri_count = ClipTriangle(v0, v1, v2, out_tris);
        
        // RASTERIZE Resulting Triangles
        for (int k = 0; k < tri_count; k++) {
            // Project the 3 vertices of the clipped triangle
            Point p0 = project_vertex(out_tris[k*3 + 0]);
            Point p1 = project_vertex(out_tris[k*3 + 1]);
            Point p2 = project_vertex(out_tris[k*3 + 2]);
            
            // Draw (using original color for now, or interpolated color if we supported it fully)
            DrawWireFrameTriangle(p0, p1, p2, obj.tca[j]);
        }
    }
    
    free(view_verts);
}

/*
 * Calculates the Z-translation needed to fit the object in the camera view.
 * * @param obj: The object to analyze
 * @param fov_degrees: The desired Field of View (e.g., 60.0 or 90.0)
 * @param padding_factor: Multiplier for extra space (e.g., 1.1 for 10% padding)
 * @return: The required Z translation in fixed-point format
 */
// renderer.c

fixed CalculateAutoPosition(Object* obj, float padding_factor) {
    
    // 1. Find Radius (Use 64-bit to be safe)
    uint64_t max_dist_sq = 0;
    for (int i = 0; i < obj->v.length; i++) {
        Vertex v = obj->v.data[i];
        int64_t x = v.x; 
        int64_t y = v.y;
        int64_t z = v.z;
        uint64_t dist = (uint64_t)(x*x) + (uint64_t)(y*y) + (uint64_t)(z*z);
        if(dist > max_dist_sq) max_dist_sq = dist;
    }

    // R is roughly 8,550,000
    fixed R = (fixed)isqrt(max_dist_sq);

    // 2. Calculate D using 64-bit math to prevent overflow
    // Formula: d = (R * 256) / 120
    // We do NOT use mul_fixed here. We treat R as a raw integer magnitude.
    
    int64_t numerator = (int64_t)R * (int64_t)FOCAL_LENGTH; // 8.5M * 256 = 2.17 Billion
    int64_t denominator = (int64_t)(CH / 2);                // 120

    // Result will be approx 18,240,000 (which fits in fixed/int32)
    fixed d = (fixed)(numerator / denominator);

    // 3. Apply Padding
    fixed padding = FLOAT_TO_FIXED(padding_factor);
    d = mul_fixed(d, padding);

    return d;
}