import sys
import math

# --- Vector Helper Functions ---
def normalize(v):
    mag = math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2])
    if mag == 0: return (0.0, 0.0, 0.0)
    return (v[0]/mag, v[1]/mag, v[2]/mag)

def cross_product(a, b):
    return (
        a[1]*b[2] - a[2]*b[1],
        a[2]*b[0] - a[0]*b[2],
        a[0]*b[1] - a[1]*b[0]
    )

def add_vectors(v1, v2):
    return (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2])

def parse_and_process(filename):
    raw_vertices = []
    faces = [] 
    
    # 1. Parse the File
    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts: continue

            if parts[0] == 'v':
                raw_vertices.append((float(parts[1]), float(parts[2]), float(parts[3])))
            
            elif parts[0] == 'f':
                face_idxs = []
                for p in parts[1:4]: 
                    idx_str = p.split('/')[0]
                    face_idxs.append(int(idx_str) - 1) 
                faces.append(face_idxs)

    # 2. Initialize Accumulators for Vertex Normals
    # List of [0,0,0] with same length as vertices
    acc_normals = [(0.0, 0.0, 0.0)] * len(raw_vertices)

    # 3. Pass 1: Accumulate Face Normals
    for face in faces:
        v0 = raw_vertices[face[0]]
        v1 = raw_vertices[face[1]]
        v2 = raw_vertices[face[2]]

        # Calculate Edge Vectors
        edge1 = (v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2])
        edge2 = (v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2])

        # Calculate Face Normal
        # Note: We do NOT normalize here yet. Weighting by area (magnitude of cross product) 
        # is actually mathematically better for irregular meshes!
        fn = cross_product(edge1, edge2)

        # Add this normal to all 3 vertices of the face
        acc_normals[face[0]] = add_vectors(acc_normals[face[0]], fn)
        acc_normals[face[1]] = add_vectors(acc_normals[face[1]], fn)
        acc_normals[face[2]] = add_vectors(acc_normals[face[2]], fn)

    # 4. Pass 2: Normalize to get Final Vertex Normals
    final_vertex_normals = []
    for n in acc_normals:
        final_vertex_normals.append(normalize(n))

    return raw_vertices, faces, final_vertex_normals

def generate_c(vertices, faces, normals, name):
    print(f"#ifndef {name.upper()}_H")
    print(f"#define {name.upper()}_H")
    print('#include "common.h"') # Assumes your Vertex struct is here

    # 1. Combined Vertex Array (Position + Normal)
    # This matches your C struct: typedef struct { float x,y,z; float nx,ny,nz; } Vertex;
    print(f"\n#define {name.upper()}_NUM_VERTS {len(vertices)}")
    print(f"const Vertex {name}_vertices[] = {{")
    
    for i in range(len(vertices)):
        v = vertices[i]
        n = normals[i]
        
        # Fixed point conversion (1.0 = 65536)
        vx, vy, vz = int(v[0]*65536), int(v[1]*65536), int(v[2]*65536)
        nx, ny, nz = int(n[0]*65536), int(n[1]*65536), int(n[2]*65536)
        
        print(f"    {{ {vx}, {vy}, {vz}, {nx}, {ny}, {nz} }},")
    
    print("};")

    # 2. Indices (The Connectivity)
    print(f"\n#define {name.upper()}_NUM_INDICES {len(faces) * 3}")
    print(f"const int {name}_indices[] = {{")
    for f in faces:
        print(f"    {f[0]}, {f[1]}, {f[2]},")
    print("};")

    print(f"#endif")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python obj_to_c.py <file.obj>")
        sys.exit(1)

    vertices, faces, vertex_normals = parse_and_process(sys.argv[1])
    name = sys.argv[1].split('.')[0]
    generate_c(vertices, faces, vertex_normals, name)