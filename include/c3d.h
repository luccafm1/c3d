/* 
 * C3D - v1.0.0 - Public Domain 3D .OBJ Renderer for Windows Terminal
 * 
 * C3D is a lightweight 3D renderer designed to display `.obj` files directly
 * within the Windows terminal. 
 * 
 * Note: C3D will **not** work on any Linux-based or non-Windows environments.
 * Current guaranteed, full support extends only to Win8-11.
 * 
 * No warranty is provided. Use at your own risk.
 * 
 * -----------------------------------------------------------------------------
 * 
 * USAGE:
 * 
 * To use C3D in your project, include this header file and define `C3D_STANDARD`
 * in **one** C or C++ source file before including `c3d.h` to generate the implementation.
 * 
 * ```c
 * // In one source file, e.g. main.c
 * #define C3D_STANDARD
 * #include "c3d.h"
 * ```
 * 
 * Then, in other source files, simply include `c3d.h` without defining `C3D_STANDARD`.
 * 
 * ```c
 * // In other source files
 * #include "c3d.h"
 * ```
 * 
 * -----------------------------------------------------------------------------
 * 
 * KNOWN LIMITATIONS:
 * 
 * - CPU-only software renderer.
 * - Windows-only support.
 * - **Only** supports `.obj` files with associated `.mtl` material files and standard image formats for textures.
 * 
 * -----------------------------------------------------------------------------
 * 
 * VERSION HISTORY:
 * 
 * v1.0.0 (2024-12-18)
 * - Initial release.
 * - Implemented core rendering pipeline.
 * 
 * v1.0.1 (2025-01-03)
 * - Shading release.
 * - Implemented Blinn-Phong shading algorithm.
 *      - Implemented linear interpolation for non-uniform scaling.
 * - STDC3DDEF definitions for compiling optimization.
 * - Optimized matrix multiplication with ASM instructions.
 * - Refactored codebase.
 */

#ifndef HEADER_H
#define HEADER_H
#endif

#ifndef C3D_H
#define C3D_H
#define BACKFACE_CULLING
#define C3D_MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <intrin.h> // asm

#ifndef _WIN32

#warning "Non-windows operating system detected."

#else

#ifdef WIN32

#include <io.h>
#define F_OK 0
#define access _access

#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>

// Define C3D_IMPLEMENTATION in one source file to include the implementation.
// Example:
// #define C3D_IMPLEMENTATION
// #include "c3d.h"
 
#ifdef C3D_STANDARD
#define C3D_IMPLEMENTATION
#define BACKFACE_CULLING
#define C3D_MENU
#define C3D_EVENT_HANDLER
#endif

#ifndef C3D_GLOBALS
#define STDC3DDEF static
#else
#define STDC3DDEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * =============================================================================
 *                              MACRO DEFINITIONS
 * =============================================================================
 */

#define C3D_RENDER_SPEED 1e-10f

#define C3D_PI 3.14159265358979323846f
#define C3D_DEG2RAD(x) ((x) * C3D_PI/180)

#define C3D_SCREEN_WIDTH 500
#define C3D_SCREEN_HEIGHT 500

#define C3D_COLOR_RGB(r, g, b) ((vec3_t){(r), (g), (b)})
#define C3D_COLOR_DEFAULT C3D_COLOR_RGB(0.0f, 0.0f, 0.0f)
#define C3D_ANSI_COLOR(R, G, B) "\033[38;2;" #R ";" #G ";" #B "m"
#define C3D_ANSI_RESET_COLOR "\033[0m"
#define C3D_SYS_ANSI_RESET system("cls"); char*ansi_ec="\033[H"; fwrite(ansi_ec, sizeof(char*), (sizeof(ansi_ec)/sizeof(ansi_ec[0])), stdout); fflush(stdout)

#define C3D_PXCHAR L'█'

#define C3D_MAX(a, b, c) (max(max(a, b), c))
#define C3D_MIN(a, b, c) (min(min(a, b), c))
#define C3D_CLAMP(var, x, y) (var > y) ? y : ((var < x) ? x : var)
#define C3D_KEY_PRESSED 0x8000
#define C3D_MOUSE_SENSITIVITY 0.01f
#define C3D_MOUSE_DELTA_SENSITIVITY 0.01f

#define _OBJ_NORMALS_MAX 10000000
#define _OBJ_TEX_MAX     10000000
#define _OBJ_VERTEX_MAX  10000000
#define _OBJ_FACES_MAX   10000000

STDC3DDEF HANDLE hConsole;
STDC3DDEF const CHAR_INFO screenBuffer[C3D_SCREEN_WIDTH * C3D_SCREEN_HEIGHT];
STDC3DDEF const SMALL_RECT consoleWriteArea = {0, 0, C3D_SCREEN_WIDTH - 1, C3D_SCREEN_HEIGHT - 1};

typedef unsigned int     c3d_intui;
typedef unsigned char    c3d_intuc;
typedef unsigned short   c3d_intus;
typedef unsigned long    c3d_intul;

/* =============================================================================
 *                                TYPE DEFINITIONS
 * =============================================================================
 */ 

// Behaviors are meant for simple, self-repeating functions
// that happen during the runtime of the program. For instance,
// mesh rotations, movements, rescaling and animations, are all
// handled through this data structure. They are utilized in
// scenes.
typedef void (*behavior_func) (struct display_t *, int, char**);
typedef enum behavior_type_t {
    C3D_CONTINUOUS_BEHAVIOR,
    C3D_STARTUP_BEHAVIOR,
} behavior_type;
typedef struct behavior_t {
    behavior_func func;
    behavior_type type;
    int argc;
    char **args;
} behavior;

// This vector is our main context for 2D vectors in space
// and the like. We use it to perform operations with texture
// coordinates most of the time, since we'll be operating within
// the context of a 3D program. 
typedef struct vec2_t{
    float x, y;
} vec2;

// A 3D vector functions as the primary data structure for
// any 3D engine. It defines positions in the x-y-z plane,
// which we use not only for spatial positioning but also
// for R-G-B vectors.
typedef struct vec3_t{
    float x, y, z;
} vec3;

// The 4D vector is the 4-dimensional vector. We use
// the conventional x-y-z-w definition. For computing
// surface normals, which are indispensible for lighting
// and shading, these are necessary.
typedef struct vec4_t {
    float x, y, z, w;
} vec4;

// A composite structure containing all information about
// a vertex in clip space. 
typedef struct vex_t{
    vec4 clip;
    vec3 space; 
    vec3 normal;
    vec2 uv;
} vex;

// A 4x4 matrix data structure.
typedef struct mat4_t {
    float m[4][4];
} mat4;

// A 3x3 matrix data structure.
typedef struct mat3_t {
    float m[3][3];
} mat3;

// Defines the triangle data structure, a polygon defined
// by 3 vectors connected in space. We include its uvmap
// & normal coordinates for mapping a texture and shading
// respectively.
typedef struct tri_t{
    vec3 vx, vy, vz;    // triangle vertices, 3D vectors as they are each contained within 3D space
    vec2 uvx, uvy, uvz; // uvmap vertices are 2D vectors as they only represent the coordinates of 2D texture
    vec3 nvx, nvy, nvz; // normal vertices are 3D vectors, because a 3D triangle must have a normal at every vertex
} tri;

// Texture structure, uses mainly stb_image to handle the
// processing of its image channels.
typedef struct texture_t{
    vec3 *data;     // rgb channels of the image
    c3d_intui width;      // image channel width
    c3d_intui height;     // image channel height
    c3d_intui channels;   // image channels
} texture;

// Material (.mtl) structure
typedef struct material_t {   
    vec3 ambient_color;     // (Ka)
    vec3 diffuse_color;     // (Kd)
    vec3 specular_color;    // (Ks)
    float shininess;        // (Ns)
    float transparency;     // (d)
    int illumination_model; // (illum)
    texture *diffuse_tex;   // (map_Kd)
    texture *specular_tex;  // (map_Ks)
    texture *normal_tex;    // (map_Bump)
} material;

// Standard material types
typedef enum c3dmtl_t {
    C3D__MTL_SOLID,     // Default, relatively unreflective and wood-like material
    C3D__MTL_PLASTIC,   // Plastic is reflective, and resembles solid in other ways
    C3D__MTL_GLASS      // Glass is not reflective, and mostly transparent
} c3d_stdmtl;

// A mesh is an array of triangles. It has a name,
// and a reference to whatever material(s) it may
// have.
typedef struct mesh_t{
    char *name;
    tri *tris;
    int tri_count;
    material *mtl;
} mesh;

// The camera defines the first person object that
// lives in the 3D space and can communicate with
// it. It mediates the player and the 3D space we
// c3d_render.
typedef struct cam_t {
    vec3 pos;       // the vector position of the camera
    mat4 matrot;    // the rotation matrix of the camera
    float theta;    // the camera's rotation angle
    float fov;      // the field of view
    float aspect;   // the aspect ratio of the camera frustum
    float fnear;    // the camera frustum's near position
    float ffar;     // the camera frustum's far position
    float yaw;      // the camera's relative Y rotation, mostly for mouse communication
    float pitch;    // like yaw, but for relative X rotation. also used for mouse communication
    float speed;    // the camera's speed
} cam;

// Light structure.
typedef struct light_t {
    vec3 position;      // the vector position of the light in 3D space
    vec3 color;         // the light's color (r, g, b)
    float brightness;   // a scalar factor that defines how strong the light is
    float radius;       // defines how far the light reaches
} light;

// The display, or scene, which is used to c3d_render
// any 3D space.
typedef struct display_t {
    mesh *meshes;               // an array contaiing all meshes to be rendered.
    vec3 background_color;      // a 3D vector representing RGB channels of the display's background color
    texture *background_tex;    // a texture for the background   
    cam camera;                 // a regular field of view camera
    behavior *behaviors;        // the behaviors of a display, actions that run every c3d_update() call
    light *lights;              // the lights of a display
    bool running;               // simple check if the display runs. Useful for constant c3d_render loops
    c3d_intus display_width;          
    c3d_intus display_height;         
    c3d_intui behavior_count;         
    c3d_intui frame_count;
    c3d_intui mesh_count;
    c3d_intui light_count;
} display;

// Window structure.
typedef struct window_t{
    c3d_intus width;
    c3d_intus height;
} window;

// Wavefront object (.obj) file structure. We
// load and store the simplest informations
// regarding a mesh first here.
typedef struct _obj_t{
    vec3 *vn;       // vector normals (vn)
    vec2 *vt;       // vector textures / uvmap coordinates (vt)
    vec3 *v;        // simple vector, defines a position in 3D space (f)
    tri  *f;        // the faces, or triangles, of an object (f)
    c3d_intul vn_size;    
    c3d_intul vt_size;
    c3d_intul v_size;
    c3d_intul f_size;
    bool smooth;    // a check for smooth shading (s)
}_obj;


STDC3DDEF c3d_intuc *c3d_strcat3(const char *prefix, const char *string, const char *suffix);
STDC3DDEF bool c3d_hastexture(const char *folder);
STDC3DDEF c3d_intui c3d_loadobjfolder(display *d, const char *folder);
STDC3DDEF void c3d_folderlist(const char *path, char ***out, int *count);
STDC3DDEF void c3d_filelist(const char *path, char ***out, int *count);
display c3d_initdisplay(cam camera, int display_width, int display_height, vec3 color);
void c3d_resetdisplay(display *d);
void c3d_lightadd(display *d, light new_light);
void c3d_meshadd(display *d, mesh new_mesh);
void c3d_behavioradd(display *d, void (*func)(display *, int, char **), behavior_type type, int argc, char **args);
STDC3DDEF void c3d_meshabs(mesh *A, mat4 B);
STDC3DDEF void c3d_meshrel(mesh *A, mat4 B);
STDC3DDEF vec3 c3d_meshcenter(mesh A);
STDC3DDEF void c3d_show_file_contents(const char *filename);



/* =============================================================================
 *                             DATA STRUCTURES
 * =============================================================================
 */



/* =============================================================================
 *                              IMPLEMENTATION
 * =============================================================================
 */

#ifdef C3D_IMPLEMENTATION

/* =============================================================================
 *                              LINEAR ALGEBRA
 * =============================================================================
 */

STDC3DDEF void c3d_vec3normalize(vec3 *vec){
    float V = sqrt(pow(vec->x, 2) + pow(vec->y, 2) + pow(vec->z, 2));
    if (V != 0){
        vec->x /= V;
        vec->y /= V;
        vec->z /= V;
    }
}

STDC3DDEF void c3d_vec4normalize(vec4 *vec){
    float V = sqrt(pow(vec->x, 2) + pow(vec->y, 2) + pow(vec->z, 2) + pow(vec->w, 2));
    if (V != 0){
        vec->x /= V;
        vec->y /= V;
        vec->z /= V;
        vec->w /= V;
    }
}

/**
 * Return the perspective projection matrix, defined by the following matrix:
 * 
 * |f/aspect, 0,          0,                          0          |
 * |   0,     f,          0,                          0          |
 * |   0,     0, (far+near)/(near-far),   (2*far*near)/(near-far)|
 * |   0,     0,         -1,                                    0|
 * 
 * https://en.wikipedia.org/wiki/Projection_matrix#:~:text=In%20statistics%2C%20the%20projection%20matrix,has%20on%20each%20fitted%20value.
 * https://en.wikipedia.org/wiki/3D_projection
 */
STDC3DDEF mat4 c3d_mat4prj(float fnear, float ffar, float fov, float aspect){
    mat4 mat;
    float f = 1.0f / tanf(0.5f *C3D_DEG2RAD(fov));
    mat.m[0][0] = f / aspect; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f;                   mat.m[0][3] = 0.0f;
    mat.m[1][0] = 0.0f;       mat.m[1][1] = f;    mat.m[1][2] = 0.0f;                   mat.m[1][3] = 0.0f;
    mat.m[2][0] = 0.0f;       mat.m[2][1] = 0.0f; mat.m[2][2] = (ffar + fnear) / (fnear - ffar); mat.m[2][3] = (2.0f * ffar * fnear) / (fnear - ffar);
    mat.m[3][0] = 0.0f;       mat.m[3][1] = 0.0f; mat.m[3][2] = -1.0f;                  mat.m[3][3] = 0.0f;

    return mat;

}

/**
 * Constructs rotation matrix around the X-axis.
 */
STDC3DDEF mat4 c3d_mat4rtx(float theta){
    mat4 mat;
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat.m[0][0] = 1.0f; mat.m[0][1] = 0.0f;        mat.m[0][2] = 0.0f;         mat.m[0][3] = 0.0f;
    mat.m[1][0] = 0.0f; mat.m[1][1] = cos_theta;   mat.m[1][2] = -sin_theta;   mat.m[1][3] = 0.0f;
    mat.m[2][0] = 0.0f; mat.m[2][1] = sin_theta;   mat.m[2][2] = cos_theta;    mat.m[2][3] = 0.0f;
    mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f;        mat.m[3][2] = 0.0f;         mat.m[3][3] = 1.0f;

    return mat;
}

/**
 * Constructs rotation matrix around the Y-axis.
 */
STDC3DDEF mat4 c3d_mat4rty(float theta){
    mat4 mat;
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat.m[0][0] = cos_theta;    mat.m[0][1] = 0.0f; mat.m[0][2] = sin_theta;   mat.m[0][3] = 0.0f;
    mat.m[1][0] = 0.0f;         mat.m[1][1] = 1.0f; mat.m[1][2] = 0.0f;        mat.m[1][3] = 0.0f;
    mat.m[2][0] = -sin_theta;   mat.m[2][1] = 0.0f; mat.m[2][2] = cos_theta;   mat.m[2][3] = 0.0f;
    mat.m[3][0] = 0.0f;         mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f;        mat.m[3][3] = 1.0f;

    return mat;
}

/**
 * Constructs rotation matrix around the Z-axis.
 */
STDC3DDEF mat4 c3d_mat4rtz(float theta){
    mat4 mat;
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat.m[0][0] = cos_theta;    mat.m[0][1] = -sin_theta;  mat.m[0][2] = 0.0f;  mat.m[0][3] = 0.0f;
    mat.m[1][0] = sin_theta;    mat.m[1][1] = cos_theta;   mat.m[1][2] = 0.0f;  mat.m[1][3] = 0.0f;
    mat.m[2][0] = 0.0f;         mat.m[2][1] = 0.0f;        mat.m[2][2] = 1.0f;  mat.m[2][3] = 0.0f;
    mat.m[3][0] = 0.0f;         mat.m[3][1] = 0.0f;        mat.m[3][2] = 0.0f;  mat.m[3][3] = 1.0f;

    return mat;
}

/**
 * Constructs translation matrix at x, y and z.
 */
STDC3DDEF mat4 c3d_mat4tra(float x, float y, float z){
    mat4 mat;

    mat.m[0][0] = 1.0f; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f; mat.m[0][3] = x;
    mat.m[1][0] = 0.0f; mat.m[1][1] = 1.0f; mat.m[1][2] = 0.0f; mat.m[1][3] = y;
    mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = 1.0f; mat.m[2][3] = z;
    mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f; mat.m[3][3] = 1.0f;

    return mat;
}

/**
 * Constructs scaling matrix at x, y and z.
 */
STDC3DDEF mat4 c3d_mat4scl(float x, float y, float z){
    mat4 mat;
    
    mat.m[0][0] = x;    mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f; mat.m[0][3] = 0.0f;
    mat.m[1][0] = 0.0f; mat.m[1][1] = y;    mat.m[1][2] = 0.0f; mat.m[1][3] = 0.0f;
    mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = z;    mat.m[2][3] = 0.0f;
    mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f; mat.m[3][3] = 1.0f;

    return mat;
}

/**
 * Multiplies two 4x4 matrices A and B.
 * 
 * https://spatial-lang.org/gemm
 */
STDC3DDEF inline mat4 c3d_mat4mul(mat4 A, mat4 B) {
    // C[i][j] = A[i][0]*B[0][j] + A[i][1]*B[1][j] + A[i][2]*B[2][j] + A[i][3]*B[3][j];
    // mat4 C;
    // for (int i = 0; i < 4; i++) {
    //     __m128 rowA = _mm_loadu_ps(&A.m[i][0]);  // A.m[i][0..3]

    //     for (int j = 0; j < 4; j++) {
    //         __m128 colB = _mm_setr_ps(
    //             B.m[0][j],
    //             B.m[1][j],
    //             B.m[2][j],
    //             B.m[3][j]
    //         );

    //         __m128 dp = _mm_dp_ps(rowA, colB, 0xFF);
    //         C.m[i][j] = _mm_cvtss_f32(dp);
    //     }
    // }
    // return C;
    mat4 C;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            C.m[i][j] = A.m[i][0]*B.m[0][j] + A.m[i][1]*B.m[1][j] + A.m[i][2]*B.m[2][j] + A.m[i][3]*B.m[3][j];
        }
    }
    return C;
}

/**
 * Multiplies 3D vector A by 4x4 matrix B and outputs a 3D vector.
 */
STDC3DDEF vec3 c3d_mat4vec3(vec3 A, mat4 B) {
    vec3 result;
    float x = A.x;
    float y = A.y;
    float z = A.z;
    result.x = B.m[0][0]*x + B.m[0][1]*y + B.m[0][2]*z + B.m[0][3];
    result.y = B.m[1][0]*x + B.m[1][1]*y + B.m[1][2]*z + B.m[1][3];
    result.z = B.m[2][0]*x + B.m[2][1]*y + B.m[2][2]*z + B.m[2][3];
    return result;
}

/**
 * Multiplies 3D vector A by a 4x4 matrix B and outputs a 4D vector.
 */
STDC3DDEF vec4 c3d_mat4vec4(vec3 A, mat4 B) {
    vec4 result;
    float x = A.x;
    float y = A.y;
    float z = A.z;
    result.x = B.m[0][0] * x + B.m[0][1] * y + B.m[0][2] * z + B.m[0][3];
    result.y = B.m[1][0] * x + B.m[1][1] * y + B.m[1][2] * z + B.m[1][3];
    result.z = B.m[2][0] * x + B.m[2][1] * y + B.m[2][2] * z + B.m[2][3];
    result.w = B.m[3][0] * x + B.m[3][1] * y + B.m[3][2] * z + B.m[3][3];
    return result;
}

/**
 * Calculates the cross product of vectors a and b.
 */
STDC3DDEF vec3 c3d_vec3cross(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

STDC3DDEF float c3d_vec3dot(vec3 A, vec3 B){
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

/**
 * Projects a 3d vector into a 2d one.
 */
STDC3DDEF vec2 c3d_project_vec3vec2(vec3 ndc, int screen_width, int screen_height) {
    vec2 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * screen_width;
    screen.y = (1.0f - ndc.y) * 0.5f * screen_height;
    return screen;
}

/**
 * Computes the inverse-transpose of a 4x4 matrix and outputs the transposed 3x3 matrix.
 * This is required for correct normal transformations under non-uniform scaling.
 */
STDC3DDEF mat3 c3d_mat4invtranspose3(mat4 M) {
    mat3 out;
    mat3 in;
    for (int i = 0; i < 3; i++) {
        for(int j=0; j<3; j++) {
            in.m[i][j] = M.m[i][j];
        }
    }

    float det = 
        in.m[0][0]*(in.m[1][1]*in.m[2][2] - in.m[1][2]*in.m[2][1]) -
        in.m[0][1]*(in.m[1][0]*in.m[2][2] - in.m[1][2]*in.m[2][0]) +
        in.m[0][2]*(in.m[1][0]*in.m[2][1] - in.m[1][1]*in.m[2][0]);

    float invDet = 1.0f / det;

    mat3 inv;
    inv.m[0][0] = (in.m[1][1]*in.m[2][2] - in.m[1][2]*in.m[2][1]) * invDet;
    inv.m[0][1] = (in.m[0][2]*in.m[2][1] - in.m[0][1]*in.m[2][2]) * invDet;
    inv.m[0][2] = (in.m[0][1]*in.m[1][2] - in.m[0][2]*in.m[1][1]) * invDet;

    inv.m[1][0] = (in.m[1][2]*in.m[2][0] - in.m[1][0]*in.m[2][2]) * invDet;
    inv.m[1][1] = (in.m[0][0]*in.m[2][2] - in.m[0][2]*in.m[2][0]) * invDet;
    inv.m[1][2] = (in.m[0][2]*in.m[1][0] - in.m[0][0]*in.m[1][2]) * invDet;

    inv.m[2][0] = (in.m[1][0]*in.m[2][1] - in.m[1][1]*in.m[2][0]) * invDet;
    inv.m[2][1] = (in.m[0][1]*in.m[2][0] - in.m[0][0]*in.m[2][1]) * invDet;
    inv.m[2][2] = (in.m[0][0]*in.m[1][1] - in.m[0][1]*in.m[1][0]) * invDet;

    for (int i = 0; i < 3; i++) {
        for(int j=0; j<3; j++) {
            out.m[i][j] = inv.m[j][i];
        }
    }

    return out;
}

/** 
 * Normal-averaging vertex normal computing
 */
STDC3DDEF void c3d_vecnormalavg(mesh *m) {
    int vertex_count = m->tri_count * 3;

    vec3 *positions = (vec3 *)malloc(sizeof(vec3)*vertex_count);
    for (int i = 0; i < m->tri_count; i++) {
        tri *t = &m->tris[i];
        positions[i*3+0] = t->vx;
        positions[i*3+1] = t->vy;
        positions[i*3+2] = t->vz;
    }

    int *map_unique = (int *)malloc(sizeof(int)*vertex_count);
    int uniqueCount = 0;
    vec3 *positions_unique = NULL;
    vec3 *sum_normals_unique = NULL;
    int *arr_count_unique = NULL;

    for (int i = 0; i < vertex_count; i++) {
        vec3 v = positions[i];
        int found = -1;
        for (int j = 0; j < uniqueCount; j++) {
            if (fabsf(positions_unique[j].x - v.x)<1e-6f &&
                fabsf(positions_unique[j].y - v.y)<1e-6f &&
                fabsf(positions_unique[j].z - v.z)<1e-6f) {
                found = j;
                break;
            }
        }
        if (found < 0) {
            positions_unique = (vec3 *)realloc(positions_unique, sizeof(vec3)*(uniqueCount+1));
            sum_normals_unique = (vec3 *)realloc(sum_normals_unique, sizeof(vec3)*(uniqueCount+1));
            arr_count_unique = (int *)realloc(arr_count_unique, sizeof(int)*(uniqueCount+1));

            positions_unique[uniqueCount] = v;
            sum_normals_unique[uniqueCount] = (vec3){0,0,0};
            arr_count_unique[uniqueCount] = 0;

            map_unique[i] = uniqueCount;
            uniqueCount++;
        } else {
            map_unique[i] = found;
        }
    }

    for (int i = 0; i < m->tri_count; i++) {
        tri *t = &m->tris[i];
        vec3 u = {t->vy.x - t->vx.x, t->vy.y - t->vx.y, t->vy.z - t->vx.z};
        vec3 v = {t->vz.x - t->vx.x, t->vz.y - t->vx.y, t->vz.z - t->vx.z};
        vec3 face_normal = c3d_vec3cross(u, v);
        c3d_vec3normalize(&face_normal);

        int i0 = map_unique[i*3+0];
        int i1 = map_unique[i*3+1];
        int i2 = map_unique[i*3+2];

        sum_normals_unique[i0].x += face_normal.x; sum_normals_unique[i0].y += face_normal.y; sum_normals_unique[i0].z += face_normal.z; arr_count_unique[i0]++;
        sum_normals_unique[i1].x += face_normal.x; sum_normals_unique[i1].y += face_normal.y; sum_normals_unique[i1].z += face_normal.z; arr_count_unique[i1]++;
        sum_normals_unique[i2].x += face_normal.x; sum_normals_unique[i2].y += face_normal.y; sum_normals_unique[i2].z += face_normal.z; arr_count_unique[i2]++;
    }

    for (int i = 0; i < uniqueCount; i++) {
        if (arr_count_unique[i] > 0) {
            sum_normals_unique[i].x /= arr_count_unique[i];
            sum_normals_unique[i].y /= arr_count_unique[i];
            sum_normals_unique[i].z /= arr_count_unique[i];
            c3d_vec3normalize(&sum_normals_unique[i]);
        }
    }

    for (int i = 0; i < m->tri_count; i++) {
        tri *t = &m->tris[i];
        int i0 = map_unique[i*3+0];
        int i1 = map_unique[i*3+1];
        int i2 = map_unique[i*3+2];

        t->nvx = sum_normals_unique[i0];
        t->nvy = sum_normals_unique[i1];
        t->nvz = sum_normals_unique[i2];
    }

    free(positions);
    free(map_unique);
    free(positions_unique);
    free(sum_normals_unique);
    free(arr_count_unique);
}

/*
 * =============================================================================
 *                          VERTEXES AND CLIPPING
 * =============================================================================
 */

/**
 * Linearly interpolate between two attributes A and B by parameter t
 */
STDC3DDEF vex c3d_lerp(vex A, vex B, float t) {
    vex R;

    R.clip.x = A.clip.x + t * (B.clip.x - A.clip.x);
    R.clip.y = A.clip.y + t * (B.clip.y - A.clip.y);
    R.clip.z = A.clip.z + t * (B.clip.z - A.clip.z);
    R.clip.w = A.clip.w + t * (B.clip.w - A.clip.w);

    R.space.x = A.space.x + t * (B.space.x - A.space.x);
    R.space.y = A.space.y + t * (B.space.y - A.space.y);
    R.space.z = A.space.z + t * (B.space.z - A.space.z);

    R.normal.x = A.normal.x + t * (B.normal.x - A.normal.x);
    R.normal.y = A.normal.y + t * (B.normal.y - A.normal.y);
    R.normal.z = A.normal.z + t * (B.normal.z - A.normal.z);

    R.uv.x = A.uv.x + t * (B.uv.x - A.uv.x);
    R.uv.y = A.uv.y + t * (B.uv.y - A.uv.y);

    return R;
}

/**
 * Intersects line segment (A,B) with the near-plane z+w=0.
 */
STDC3DDEF vex c3d_nearintersect(vex A, vex B) {
    float Ad = A.clip.z + A.clip.w;
    float Bd = B.clip.z + B.clip.w;

    float t = Ad / (Ad - Bd); 

    return c3d_lerp(A, B, t);
}

/**
 * Helper function for finding out whether a vertex v is in the near clip plane.
 */
STDC3DDEF bool c3d_innear(vex v) {
    return (v.clip.z + v.clip.w) >= 0.0f;
}

/**
 * Sutherland–Hodgman clipping against the near plane (z+w=0).
 */
STDC3DDEF int c3d_suthhodgman(vex *inVerts, int inCount, vex *outVerts) {
    vex temp[8];  
    int outCount = 0;

    for (int i = 0; i < inCount; i++) {
        vex current = inVerts[i];
        vex next = inVerts[(i+1)%inCount];
        bool currInside = c3d_innear(current);
        bool nextInside = c3d_innear(next);

        if (currInside && nextInside) {
            temp[outCount++] = next;
        } 
        if (currInside && !nextInside) {
            temp[outCount++] = c3d_nearintersect(current, next);
        }
        if (!currInside && nextInside) {
            temp[outCount++] = c3d_nearintersect(current, next);
            temp[outCount++] = next;
        } 
    }

    for (int i = 0; i < outCount; i++) {
        outVerts[i] = temp[i];
    }
    return outCount;
}

/**
 * Handles clipping against near-plane (w = 0)
 */
STDC3DDEF int c3d_nearclip(tri t, mat4 matcam, tri *clippedtri) {
    vex in[3];
    vec3 positions[3] = { t.vx, t.vy, t.vz };
    vec3 normals[3]   = { t.nvx, t.nvy, t.nvz };
    vec2 uvs[3]       = { t.uvx, t.uvy, t.uvz };

    for (int i = 0; i < 3; i++) {
        vec4 clip = c3d_mat4vec4(positions[i], matcam);
        in[i].clip = clip;
        in[i].space = positions[i];
        in[i].normal = normals[i];
        in[i].uv = uvs[i];
    }

    vex out[8];
    int outc = c3d_suthhodgman(in, 3, out);

    if (outc < 3) {
        return 0;
    }

    if (outc == 3) {
        tri result;
        result.vx = out[0].space;
        result.vy = out[1].space;
        result.vz = out[2].space;

        result.nvx = out[0].normal;
        result.nvy = out[1].normal;
        result.nvz = out[2].normal;

        result.uvx = out[0].uv;
        result.uvy = out[1].uv;
        result.uvz = out[2].uv;

        clippedtri[0] = result;
        return 1;
    } else if (outc == 4) {
        tri r1, r2;
        r1.vx =     out[0].space;       r1.vy = out[1].space;     r1.vz = out[2].space;
        r1.nvx =    out[0].normal;      r1.nvy = out[1].normal;   r1.nvz = out[2].normal;
        r1.uvx =    out[0].uv;          r1.uvy = out[1].uv;       r1.uvz = out[2].uv;

        r2.vx =     out[0].space;       r2.vy = out[2].space;       r2.vz = out[3].space;
        r2.nvx =    out[0].normal;      r2.nvy = out[2].normal;     r2.nvz = out[3].normal;
        r2.uvx =    out[0].uv;          r2.uvy = out[2].uv;         r2.uvz = out[3].uv;

        clippedtri[0] = r1;
        clippedtri[1] = r2;
        return 2;
    }

    return 0;
}

/**
 * Edge function for rasterization
 */
STDC3DDEF float c3d_edge(vec2 v0, vec2 v1, vec2 v2){
    return (((v2.x-v0.x)*(v1.y-v0.y)) - ((v1.x-v0.x)*(v2.y-v0.y)));
}

#ifdef BACKFACE_CULLING
/**
 * Calculates whether a vector is in view.
 * 
 * We will be discarding all triangles where the dot product of their surface normal and the 
 * camera-to-triangle vector is greater than or equal to zero.
 */
STDC3DDEF bool c3d_backface(tri t, vec3 pos){
    vec3 u, v;
    u.x = t.vy.x - t.vx.x;
    u.y = t.vy.y - t.vx.y;
    u.z = t.vy.z - t.vx.z;

    v.x = t.vz.x - t.vx.x;
    v.y = t.vz.y - t.vx.y;
    v.z = t.vz.z - t.vx.z;

    vec3 n = c3d_vec3cross(u, v);
    vec3 view = {t.vx.x - pos.x, t.vx.y - pos.y, t.vx.z - pos.z};
    
    float dot = c3d_vec3dot(n, view);

    return (dot >= 0.0f);
}

#endif

/*
 * =============================================================================
 *               SHADING, TEXTURING, RENDERING AND RASTERIZING
 * =============================================================================
 */

/**
 * Function for displaying all buffers.
 * 
 * This function uses the ANSI escape character \033[ and defines
 * 38;2 for coloring on the screen, and the last three digits,
 * separated by the semicolons, are the R, G and B, 48;2 for bg
 * coloring.
 */ 
STDC3DDEF void c3d_render(display *d, wchar_t **buffer, COLORREF **colorBuffer) {
    wchar_t *output_buffer = NULL;
    size_t output_buffer_size = 0;

    if (!output_buffer) {
        size_t max_line_length = d->display_width * 30 + 10;
        output_buffer_size = d->display_height * max_line_length * sizeof(wchar_t);
        output_buffer = (wchar_t *)malloc(output_buffer_size);
        if (!output_buffer) {
            fwprintf(stderr, L"Memory allocation failed for output_buffer.\n");
            exit(EXIT_FAILURE);
        }
    }

    size_t buffer_pos = 0;

    int bgr = (int)(d->background_color.x);
    int bgg = (int)(d->background_color.y);
    int bgb = (int)(d->background_color.z);
    
    bgr = C3D_CLAMP(bgr, 0, 255);
    bgg = C3D_CLAMP(bgg, 0, 255);
    bgb = C3D_CLAMP(bgb, 0, 255);
    
    buffer_pos += swprintf(&output_buffer[buffer_pos], output_buffer_size - buffer_pos, L"\x1b[48;2;%d;%d;%dm", bgr, bgg, bgb);
    buffer_pos += swprintf(&output_buffer[buffer_pos], output_buffer_size - buffer_pos, L"\x1b[H");

    COLORREF lastColor = 0xFFFFFFFF; 

    for (int y = 0; y < d->display_height; y++) {
        for (int x = 0; x < d->display_width; x++) {
            COLORREF color = colorBuffer[y][x];

            if (color != lastColor) {
                int r = GetRValue(color);
                int g = GetGValue(color);
                int b = GetBValue(color);
                buffer_pos += swprintf(&output_buffer[buffer_pos], output_buffer_size - buffer_pos, L"\x1b[38;2;%d;%d;%dm", r, g, b);
                lastColor = color;
            }
            
            wchar_t wc = buffer[y][x];
            output_buffer[buffer_pos++] = wc;
        }
        output_buffer[buffer_pos++] = L'\n';
    }

    buffer_pos += swprintf(&output_buffer[buffer_pos], output_buffer_size - buffer_pos, L"\x1b[0m");

    DWORD written;
    WriteConsoleW(hConsole, output_buffer, (DWORD)buffer_pos, &written, NULL);
}

/**
 * Compute illumination for a given material using Blinn-Phong's algorithm.
 * 
 * https://en.wikipedia.org/wiki/Blinn–Phong_reflection_model
 */
STDC3DDEF void c3d_bphongshade(display *d, vec3 normal, vec3 space, material *mtl, vec3 *out_ambient, vec3 *out_diffuse, vec3 *out_specular) {
    *out_ambient = mtl->ambient_color; 
    out_diffuse->x = 0.0f; out_diffuse->y = 0.0f; out_diffuse->z = 0.0f;
    out_specular->x = 0.0f; out_specular->y = 0.0f; out_specular->z = 0.0f;

    for (int i = 0; i < d->light_count; i++) {
        light L = d->lights[i];

        vec3 toLight = {L.position.x - space.x, L.position.y - space.y, L.position.z - space.z};
        float dist = sqrtf(toLight.x*toLight.x + toLight.y*toLight.y + toLight.z*toLight.z);
        if (dist <= 0.0001f) dist = 0.0001f;
        c3d_vec3normalize(&toLight);

        float NdotL = max(0.0f, c3d_vec3dot(normal, toLight));
        if (NdotL > 0.0f) {
            if (dist > L.radius) continue; 
            float attenuation = 1.0f / (1.0f + (dist / L.radius) * (dist / L.radius));

            vec3 viewDir = {d->camera.pos.x - space.x, d->camera.pos.y - space.y, d->camera.pos.z - space.z};
            c3d_vec3normalize(&viewDir);

            vec3 halfDir = {viewDir.x + toLight.x, viewDir.y + toLight.y, viewDir.z + toLight.z};
            c3d_vec3normalize(&halfDir);

            float NdotH = max(0.0f, c3d_vec3dot(normal, halfDir));
            float specular_factor = powf(NdotH, mtl->shininess);

            out_diffuse->x += mtl->diffuse_color.x * L.color.x * L.brightness;
            out_diffuse->y += mtl->diffuse_color.y * L.color.y * L.brightness;
            out_diffuse->z += mtl->diffuse_color.z * L.color.z * L.brightness;

            out_specular->x += mtl->specular_color.x * L.color.x * L.brightness * specular_factor * attenuation;
            out_specular->y += mtl->specular_color.y * L.color.y * L.brightness * specular_factor * attenuation;
            out_specular->z += mtl->specular_color.z * L.color.z * L.brightness * specular_factor * attenuation;
        }
    }

    out_ambient->x = C3D_CLAMP(out_ambient->x, 0.0f, 1.0f);
    out_ambient->y = C3D_CLAMP(out_ambient->y, 0.0f, 1.0f);
    out_ambient->z = C3D_CLAMP(out_ambient->z, 0.0f, 1.0f);

    out_diffuse->x = C3D_CLAMP(out_diffuse->x, 0.0f, 1.0f);
    out_diffuse->y = C3D_CLAMP(out_diffuse->y, 0.0f, 1.0f);
    out_diffuse->z = C3D_CLAMP(out_diffuse->z, 0.0f, 1.0f);

    out_specular->x = C3D_CLAMP(out_specular->x, 0.0f, 1.0f);
    out_specular->y = C3D_CLAMP(out_specular->y, 0.0f, 1.0f);
    out_specular->z = C3D_CLAMP(out_specular->z, 0.0f, 1.0f);
}

/**
 * Samples a texture
 */
STDC3DDEF vec3 c3d_texsample(texture *tex, float u, float v) {
    if (tex->data == NULL) {
        vec3 default_color = {1.0f, 1.0f, 1.0f};
        return default_color;
    }

    u = C3D_CLAMP(u, 0.0f, 1.0f);
    v = C3D_CLAMP(v, 0.0f, 1.0f);

    int tex_x = (int)(u * (tex->width - 1));
    int tex_y = (int)((1.0f - v) * (tex->height - 1)); 

    int index = tex_y * tex->width + tex_x;

    vec3 color = tex->data[index];

    color.x = C3D_CLAMP(color.x, 0.0f, 1.0f);
    color.y = C3D_CLAMP(color.y, 0.0f, 1.0f);
    color.z = C3D_CLAMP(color.z, 0.0f, 1.0f);

    return color;
}

STDC3DDEF void c3d_bresenham(display *d, wchar_t **buffer, wchar_t **colorBuffer, vec2 v0, vec2 v1){

    int x1 = v0.x;
    int y1 = v0.y;

    int x2 = v1.x;
    int y2 = v1.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x1 >= 0 && x1 < d->display_width && y1 >= 0 && y1 < d->display_height) {
            COLORREF color = RGB(255, 255, 255);
            buffer[y1][x1] = C3D_PXCHAR;
            colorBuffer[y1][x1] = color;
        }

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * Algorithm to fill triangles on any given display.
 */
STDC3DDEF void c3d_rasterize(display *d, wchar_t **buffer, COLORREF **colorBuffer, float **depthBuffer,
               vec3 v0_ndc, vec3 v1_ndc, vec3 v2_ndc,
               float w0_clip, float w1_clip, float w2_clip,
               tri t, material *mtl) {

    vec2 pv0 = c3d_project_vec3vec2(v0_ndc, d->display_width, d->display_height);
    vec2 pv1 = c3d_project_vec3vec2(v1_ndc, d->display_width, d->display_height);
    vec2 pv2 = c3d_project_vec3vec2(v2_ndc, d->display_width, d->display_height);

    int minx = max(C3D_MIN(pv0.x, pv1.x, pv2.x), 0);
    int maxx = min(C3D_MAX(pv0.x, pv1.x, pv2.x), d->display_width - 1);
    int miny = max(C3D_MIN(pv0.y, pv1.y, pv2.y), 0);
    int maxy = min(C3D_MAX(pv0.y, pv1.y, pv2.y), d->display_height - 1);

    float area = c3d_edge(pv0, pv1, pv2);
    if (area == 0) return;

    float inv_w0 = 1.0f / w0_clip;
    float inv_w1 = 1.0f / w1_clip;
    float inv_w2 = 1.0f / w2_clip;

    vec3 wPos0 = t.vx; 
    vec3 wPos1 = t.vy;
    vec3 wPos2 = t.vz;

    vec3 n0 = t.nvx;
    vec3 n1 = t.nvy;
    vec3 n2 = t.nvz;

    vec2 uv0 = t.uvx;
    vec2 uv1 = t.uvy;
    vec2 uv2 = t.uvz;

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            vec2 vxy = {x + 0.5f, y + 0.5f};
            float w0 = c3d_edge(pv1, pv2, vxy) / area;
            float w1 = c3d_edge(pv2, pv0, vxy) / area;
            float w2 = c3d_edge(pv0, pv1, vxy) / area;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                float denom = w0 * inv_w0 + w1 * inv_w1 + w2 * inv_w2;
                if (denom == 0.0f) continue;
                float z = (v0_ndc.z * w0 / w0_clip + v1_ndc.z * w1 / w1_clip + v2_ndc.z * w2 / w2_clip) / denom ;

                if (z < depthBuffer[y][x]) {
                    depthBuffer[y][x] = z;

                    float u = (uv0.x*inv_w0*w0 + uv1.x*inv_w1*w1 + uv2.x*inv_w2*w2) / denom;
                    float v = (uv0.y*inv_w0*w0 + uv1.y*inv_w1*w1 + uv2.y*inv_w2*w2) / denom;

                    vec3 space;
                    space.x = (wPos0.x*inv_w0*w0 + wPos1.x*inv_w1*w1 + wPos2.x*inv_w2*w2) / denom;
                    space.y = (wPos0.y*inv_w0*w0 + wPos1.y*inv_w1*w1 + wPos2.y*inv_w2*w2) / denom;
                    space.z = (wPos0.z*inv_w0*w0 + wPos1.z*inv_w1*w1 + wPos2.z*inv_w2*w2) / denom;

                    vec3 normal;
                    normal.x = (n0.x * inv_w0 * w0 + n1.x * inv_w1 * w1 + n2.x * inv_w2 * w2) / denom;
                    normal.y = (n0.y * inv_w0 * w0 + n1.y * inv_w1 * w1 + n2.y * inv_w2 * w2) / denom;
                    normal.z = (n0.z * inv_w0 * w0 + n1.z * inv_w1 * w1 + n2.z * inv_w2 * w2) / denom;
                    c3d_vec3normalize(&normal);

                    vec3 ambient, diffuse, specular;
                    c3d_bphongshade(d, normal, space, mtl, &ambient, &diffuse, &specular);

                    vec3 tex_color = c3d_texsample(mtl->diffuse_tex, u, v);

                    vec3 final_color;
                    final_color.x = (ambient.x + diffuse.x) * tex_color.x + specular.x;
                    final_color.y = (ambient.y + diffuse.y) * tex_color.y + specular.y;
                    final_color.z = (ambient.z + diffuse.z) * tex_color.z + specular.z;

                    final_color.x = (1.0f - mtl->transparency) * d->background_color.x + mtl->transparency * final_color.x;
                    final_color.y = (1.0f - mtl->transparency) * d->background_color.y + mtl->transparency * final_color.y;
                    final_color.z = (1.0f - mtl->transparency) * d->background_color.z + mtl->transparency * final_color.z;

                    final_color.x = C3D_CLAMP(final_color.x, 0.0f, 1.0f);
                    final_color.y = C3D_CLAMP(final_color.y, 0.0f, 1.0f);
                    final_color.z = C3D_CLAMP(final_color.z, 0.0f, 1.0f);

                    COLORREF color = RGB(
                        (int)(final_color.x * 255.0f),
                        (int)(final_color.y * 255.0f),
                        (int)(final_color.z * 255.0f)
                    );

                    buffer[y][x] = C3D_PXCHAR;
                    colorBuffer[y][x] = color;
                }
            }
        }
    }
}

/**
 * Updates all buffers.
 */
void c3d_update(display* d){
    for (int i = 0; i < d->behavior_count; i++) {
        behavior *b = &d->behaviors[i];
        if (b->func != NULL) {
            switch (b->type){
                case C3D_CONTINUOUS_BEHAVIOR: b->func(d, b->argc, b->args); break;
                case C3D_STARTUP_BEHAVIOR: (d->frame_count == 0) ? b->func(d, b->argc, b->args) : NULL; break;
            }
        }
    }
    float** depthBuffer = (float**)malloc(d->display_height * sizeof(float*));
    wchar_t** buffer = (wchar_t**)malloc(d->display_height * sizeof(wchar_t*));
    COLORREF** colorBuffer = (COLORREF**)malloc(d->display_height * sizeof(COLORREF*));
    
    for (int i = 0; i < d->display_height; i++) {
        depthBuffer[i] = (float*)malloc(d->display_width * sizeof(float));
        buffer[i] = (wchar_t*)malloc(d->display_width * sizeof(wchar_t));
        colorBuffer[i] = (COLORREF*)malloc(d->display_width * sizeof(COLORREF));
        
        for (int j = 0; j < d->display_width; j++) {
            buffer[i][j] = L' ';
            colorBuffer[i][j] = RGB(0, 0, 0);
            depthBuffer[i][j] = INFINITY;
        }
    }

    mat4 matproj = c3d_mat4prj(d->camera.fnear, d->camera.ffar, d->camera.fov, d->camera.aspect);
    mat4 camtranslate = c3d_mat4tra(-d->camera.pos.x, -d->camera.pos.y, -d->camera.pos.z);
    mat4 camview = c3d_mat4mul(d->camera.matrot, camtranslate);
    mat4 matcam = c3d_mat4mul(matproj, camview);

    for (int i = 0; i < d->mesh_count; i++) {
        mesh* m = &d->meshes[i];
        for (int j = 0; j < m->tri_count; j++) {
            tri t = m->tris[j];

            tri t_clipped[2];
            int tri_count = c3d_nearclip(t, matcam, t_clipped);

            for (int c = 0; c < tri_count; c++) {
                tri t = t_clipped[c];

                #ifdef BACKFACE_CULLING
                if (c3d_backface(t, d->camera.pos)) continue;
                #endif

                vec4 v0_clip = c3d_mat4vec4(t.vx, matcam);
                vec4 v1_clip = c3d_mat4vec4(t.vy, matcam);
                vec4 v2_clip = c3d_mat4vec4(t.vz, matcam);

                vec3 v0_ndc = {v0_clip.x / v0_clip.w, v0_clip.y / v0_clip.w, v0_clip.z / v0_clip.w};
                vec3 v1_ndc = {v1_clip.x / v1_clip.w, v1_clip.y / v1_clip.w, v1_clip.z / v1_clip.w};
                vec3 v2_ndc = {v2_clip.x / v2_clip.w, v2_clip.y / v2_clip.w, v2_clip.z / v2_clip.w};

                if ((v0_ndc.x < -1.0f && v1_ndc.x < -1.0f && v2_ndc.x < -1.0f) ||
                    (v0_ndc.x >  1.0f && v1_ndc.x >  1.0f && v2_ndc.x >  1.0f) ||
                    (v0_ndc.y < -1.0f && v1_ndc.y < -1.0f && v2_ndc.y < -1.0f) ||
                    (v0_ndc.y >  1.0f && v1_ndc.y >  1.0f && v2_ndc.y >  1.0f) ||
                    (v0_ndc.z < -1.0f && v1_ndc.z < -1.0f && v2_ndc.z < -1.0f) ||
                    (v0_ndc.z >  1.0f && v1_ndc.z >  1.0f && v2_ndc.z >  1.0f)) {
                    continue;
                }

                #ifndef NO_FILL
                c3d_rasterize(d, buffer, colorBuffer, depthBuffer, v0_ndc, v1_ndc, v2_ndc, v0_clip.w, v1_clip.w, v2_clip.w, t, m->mtl);
                #else
                // c3d_bresenham()
                #endif
            }
        }
    }

    d->frame_count++;
    c3d_render(d, buffer, colorBuffer);
    for (int i = 0; i < d->display_height; i++) {
        free(buffer[i]);
        free(colorBuffer[i]);
        free(depthBuffer[i]);
    }
    free(buffer);
    free(colorBuffer);
    free(depthBuffer);
}

/*
 * ==============================================================================
 *                      MESH, TEXTURE AND OBJECT LOADERS
 * =============================================================================
 */

STDC3DDEF texture c3d_generic_texgen(){
    int width = 120; 
    int height = 120;
    int channels = 3;
    
    texture tex;
    tex.width = width;
    tex.height = height;
    tex.channels = channels;
    tex.data = (vec3 *)malloc(width * height * sizeof(vec3));
    
    if (tex.data == NULL) {
        fprintf(stderr, "Failed to allocate memory for placeholder texture.\n");
        exit(EXIT_FAILURE);
    }
    
    int checker_size = 4; 
    
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            if (((x / checker_size) + (y / checker_size)) % 2 == 0) {
                tex.data[y * width + x] = (vec3){0.5f, 0.0f, 0.5f};
            } else {
                tex.data[y * width + x] = (vec3){0.0f, 0.0f, 0.0f};
            }
        }
    }
    
    return tex;
}

STDC3DDEF void c3d_init__obj(_obj *o){
    o->v  = (vec3 *)malloc(_OBJ_VERTEX_MAX * sizeof(vec3));
    o->vn = (vec3 *)malloc(_OBJ_NORMALS_MAX * sizeof(vec3));
    o->vt = (vec2 *)malloc(_OBJ_TEX_MAX * sizeof(vec2));
    o->f  = (tri *)malloc(_OBJ_FACES_MAX * sizeof(tri));

    o->v_size  = 0;
    o->vn_size = 0;
    o->vt_size = 0;
    o->f_size  = 0;
}

STDC3DDEF _obj *c3d_loadobj(const char *path) {  
    FILE *f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "Path %s has no valid .OBJ file.", path);
        return;
    }

    _obj *obj = malloc(sizeof(_obj));
    
    c3d_init__obj(obj);

    char line[1024];
    size_t n;
    int i = 0;

    while (fgets(line, sizeof(line), f)){
        n++;

        if (line[0] == '\n' || line[0] == '\r') continue; // whitespace
        if (line[0] == 'g') {} // WIP texture groups 

        // smooth shading
        if (line[0] == 's' && line[1] == ' ') {
            if (line[2] == '1' || (line[2] == 'o' && line[3] == 'n')) obj->smooth = true;
            if (line[2] == '0' || (line[2] == 'o' && line[3] == 'f' && line[4] == 'f')) obj->smooth = false;
        }

        // UV coordinates
        if (line[0] == 'v' && line[1] == 't') { 
            vec2 uv;
            sscanf(line + 2, "%f %f", &uv.x, &uv.y);
            obj->vt[obj->vt_size++] = (vec2){uv.x, uv.y};
        } 

        // Normal coordinates
        if (line[0] == 'v' && line[1] == 'n') { 
            vec3 n;
            sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
            obj->vn[obj->vn_size++] = (vec3){n.x, n.y, n.z}; 
        } 

        // 3D vertices
        if (line[0] == 'v' && line[1] == ' ') { 
            vec3 v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            obj->v[obj->v_size++] = (vec3){v.x, v.y, v.z};
        }

        // Faces: triangle or quads
        if (line[0] == 'f') {
            int vidx[4] = {0};
            int vtidx[4]= {0};
            int vnidx[4]= {0};

            int c = 0;
            char *token = strtok(line + 2, " \t\r\n"); 
            while (token != NULL && c < 4){
                int v = 0;
                int t = 0;
                int n = 0;
                if (sscanf(token, "%d/%d/%d", &v, &t, &n) == 3) {
                    vidx[c] = v;
                    vtidx[c] = t;
                    vnidx[c] = n;
                } else if (sscanf(token, "%d//%d", &v, &n) == 2) {
                    vidx[c] = v;
                    vtidx[c] = 0;
                    vnidx[c] = n;
                } else if (sscanf(token, "%d/%d", &v, &t) == 2) {
                    vidx[c] = v;
                    vtidx[c] = t;
                    vnidx[c] = 0;
                } else {
                    sscanf(token, "%d", &v);
                    vidx[c] = v;
                    vtidx[c] = 0;
                    vnidx[c] = 0;
                }
                c++;
                token = strtok(NULL, " \t\r\n");
            }

            for (int i = 1; i < c - 1; i++){
                tri triangle;

                triangle.vx = obj->v[vidx[0] - 1];
                triangle.vy = obj->v[vidx[i] - 1];
                triangle.vz = obj->v[i+1 < c ? vidx[i+1] - 1 : vidx[0] - 1];

                if (obj->vt_size > 0){
                    if (vtidx[0] > 0 && vtidx[i] > 0 && vtidx[i+1] > 0){
                        triangle.uvx = obj->vt[vtidx[0]-1];
                        triangle.uvy = obj->vt[vtidx[i]-1];
                        triangle.uvz = obj->vt[i+1 < c ? vtidx[i+1] - 1 : vtidx[0] - 1];
                    } else {
                        triangle.uvx = (vec2){0, 0};
                        triangle.uvy = (vec2){0, 0};
                        triangle.uvz = (vec2){0, 0};
                    }
                } else {
                    triangle.uvx = (vec2){0, 0};
                    triangle.uvy = (vec2){0, 0};
                    triangle.uvz = (vec2){0, 0};
                }

                if (obj->vn_size > 0){
                    if (vnidx[0] > 0 && vnidx[i] > 0 && vnidx[i+1] > 0){
                        triangle.nvx = obj->vn[vnidx[0]-1];
                        triangle.nvy = obj->vn[vnidx[i]-1];
                        triangle.nvz = obj->vn[i+1 < c ? vnidx[i+1] - 1 : vnidx[0] - 1];
                    } else {
                        triangle.nvx = (vec3){0,0,0};
                        triangle.nvy = (vec3){0,0,0};
                        triangle.nvz = (vec3){0,0,0};
                    }
                } else {
                    triangle.nvx = (vec3){0,0,0};
                    triangle.nvy = (vec3){0,0,0};
                    triangle.nvz = (vec3){0,0,0};
                }

                obj->f[obj->f_size++] = triangle;
            }
        } 
    }   

    return obj;
} 

STDC3DDEF texture c3d_loadimg(const char *path) {
    texture tex;
    int w, h, comp;
    char *img = (char*)stbi_load(path, &w, &h, &comp, 3);

    if (img == NULL) {
        fprintf(stderr, "Failed to load image at path: %s\n", path);
        tex.width = 0;
        tex.height = 0;
        tex.channels = 0;
        tex.data = NULL;
        return tex;
    }

    tex.width = w;
    tex.height = h;
    tex.channels = 3;
    tex.data = (vec3 *)malloc(w * h * sizeof(vec3));

    if (tex.data == NULL) {
        fprintf(stderr, "Memory allocation failed for texture data.\n");
        stbi_image_free(img);
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < w * h; i++) {
        tex.data[i].x = ((c3d_intuc)img[i * 3 + 0]) / 255.0f;
        tex.data[i].y = ((c3d_intuc)img[i * 3 + 1]) / 255.0f;
        tex.data[i].z = ((c3d_intuc)img[i * 3 + 2]) / 255.0f;

    }
    
    stbi_image_free(img);
    return tex;
}

STDC3DDEF void c3d_trim_whitespace(char *str) {
    char *end;
    while(isspace((c3d_intuc)*str)) str++;
    if(*str == 0) return; 
    end = str + strlen(str) - 1;
    while(end > str && isspace((c3d_intuc)*end)) end--;
    *(end+1) = 0;
}

#define MAX_PATH_LENGTH 256

/**
 * Parse an MTL file and load all materials.
 */
STDC3DDEF material *c3d_loadmtl(const char *path, int *material_count) {
    FILE *file = fopen(path, "r");
    if (!file) {
        *material_count = 0;
        return NULL;
    }

    char line[1024];
    material *materials = NULL;
    material *current_material = NULL;
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        c3d_trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        if (strncmp(line, "newmtl", 6) == 0) {
            count++;
            materials = (material *)realloc(materials, sizeof(material) * count);
            if (!materials) {
                perror("Failed to allocate memory for materials");
                exit(EXIT_FAILURE);
            }
            current_material = &materials[count - 1];
            current_material->diffuse_tex = NULL;
            current_material->specular_tex = NULL;
            current_material->normal_tex = NULL;
            current_material->ambient_color = (vec3){0.2f, 0.2f, 0.2f};
            current_material->diffuse_color = (vec3){0.8f, 0.8f, 0.8f};
            current_material->specular_color = (vec3){1.0f, 1.0f, 1.0f};
            current_material->shininess = 32.0f; 
            current_material->transparency = 1.0f;
            current_material->illumination_model = 2;
        } 
        if (strncmp(line, "Ka ", 3) == 0) {
            sscanf(line + 3, "%f %f %f", &current_material->ambient_color.x, &current_material->ambient_color.y, &current_material->ambient_color.z);
        }
        if (strncmp(line, "Kd ", 3) == 0) {
            sscanf(line + 3, "%f %f %f", &current_material->diffuse_color.x, &current_material->diffuse_color.y, &current_material->diffuse_color.z);
        }
        if (strncmp(line, "Ks ", 3) == 0) {
            sscanf(line + 3, "%f %f %f", &current_material->specular_color.x, &current_material->specular_color.y, &current_material->specular_color.z);
        }
        if (strncmp(line, "Ns ", 3) == 0) {
            sscanf(line + 3, "%f", &current_material->shininess);
        }
        if (strncmp(line, "d ", 2) == 0) {
            sscanf(line + 2, "%f", &current_material->transparency);
        }
        if (strncmp(line, "map_Kd ", 7) == 0) {
            char texture_path[MAX_PATH_LENGTH];
            sscanf(line + 7, "%s", texture_path);
            texture temp = c3d_loadimg(texture_path);
            if (temp.data != NULL) {
                current_material->diffuse_tex = malloc(sizeof(texture));
                *current_material->diffuse_tex = temp;
            }
        }
        if (strncmp(line, "map_Ks ", 7) == 0) {
            char texture_path[MAX_PATH_LENGTH];
            sscanf(line + 7, "%s", texture_path);
            texture temp = c3d_loadimg(texture_path);
            if (temp.data != NULL) {
                current_material->specular_tex = malloc(sizeof(texture));
                *current_material->specular_tex = temp;
            }
        }
        if (strncmp(line, "map_Bump", 8) == 0 || strncmp(line, "map_bump", 8) == 0) {
            char texture_path[MAX_PATH_LENGTH];
            sscanf(line + 8, "%s", texture_path);
            texture temp = c3d_loadimg(texture_path);
            if (temp.data != NULL) {
                current_material->normal_tex = malloc(sizeof(texture));
                *current_material->normal_tex = temp;
            }
        }
    }

    fclose(file);
    *material_count = count;
    
    return materials;
}

STDC3DDEF mesh c3d_loadmesh(const char *dir) {
    char **filepaths;
    int count;
    char *fulldir = c3d_strcat3(dir, "/*", "");
    
    c3d_filelist(fulldir, &filepaths, &count);

    char *obj = ".OBJ";
    char *mtl = ".MTL";
    char *png = ".PNG"; 
    char *jpg = ".JPG";

    char *objpath = NULL;
    char *mtlpath = NULL;
    char *diffpath = NULL;

    int idx = 0;
    int objc = 0, mtlc = 0;
    while (idx < count){
        char *filepath = filepaths[idx];
        for (int i = 0; filepath[i] != '\0'; i++) {
            filepath[i] = toupper(filepath[i]);
        }

        if (strstr(filepath, obj) != NULL){
            char *full_obj_path = c3d_strcat3(dir, "/", filepaths[idx]);
            objpath = malloc(strlen(full_obj_path) + 1);
            strcpy(objpath, full_obj_path);
            free(full_obj_path); 

            objc++;
        }
        if (strstr(filepath, mtl) != NULL){
            char *full_mtl_path = c3d_strcat3(dir, "/", filepaths[idx]);
            mtlpath = malloc(strlen(full_mtl_path) + 1);
            strcpy(mtlpath, full_mtl_path);
            free(full_mtl_path);

            mtlc++;
        }
        if (strstr(filepath, png) != NULL || strstr(filepath, jpg) != NULL){
            char *full_diff_path = c3d_strcat3(dir, "/", filepaths[idx]);
            diffpath = malloc(strlen(full_diff_path) + 1);
            strcpy(diffpath, full_diff_path);
            free(full_diff_path);
        }

        idx++;
    }
    
    if (objc == 0){
        fprintf(stderr, "FATAL: No .OBJ files found at path %s. Exiting...", dir);
        exit(-1);
    }

    if (objc > 1){
        fprintf(stderr, "RUN-TIME WARNING: More than one .OBJ file found at path `%s`. Last found path (%s) will be used.", dir, objpath);
    }

    if (mtlc > 1){
        fprintf(stderr, "RUN-TIME WARNING: More than one .MTL file found at path `%s`. Last found path (%s) will be used.", dir, mtlpath);
    }

    _obj *lobj = c3d_loadobj(objpath);

    mesh new_mesh;
    new_mesh.tris = (tri *)malloc(lobj->f_size * sizeof(tri));
    if (!new_mesh.tris) {
        perror("FATAL: Failed to allocate memory for mesh faces.");
        exit(-1);
    }
    memcpy(new_mesh.tris, lobj->f, lobj->f_size * sizeof(tri));
    new_mesh.tri_count = (int)lobj->f_size;

    new_mesh.name = malloc(sizeof(char));

    #ifndef FORCE_SMOOTH
    if (lobj->smooth) {
        c3d_vecnormalavg(&new_mesh);
    }
    #else
    c3d_vecnormalavg(&new_mesh);
    #endif

    int materialc = 0;
    material *materials = NULL;
    if (mtlpath != NULL){
        materials = c3d_loadmtl(mtlpath, &materialc);
    }
    
    texture tex;
    if (diffpath != NULL){
        tex = c3d_loadimg(diffpath);
    } else {
        tex = c3d_generic_texgen();
    }

    if (materials != NULL) {
        new_mesh.mtl = (material *)malloc(sizeof(material));
        if (new_mesh.mtl == NULL) {
            fprintf(stderr, "Memory allocation failed for material.\n");
            exit(EXIT_FAILURE);
        }
        *new_mesh.mtl = materials[0];
        
        if (new_mesh.mtl->diffuse_tex == NULL) {
            new_mesh.mtl->diffuse_tex = (texture *)malloc(sizeof(texture));
            if (new_mesh.mtl->diffuse_tex == NULL) {
                fprintf(stderr, "Memory allocation failed for diffuse_tex.\n");
                exit(EXIT_FAILURE);
            }
            *new_mesh.mtl->diffuse_tex = tex;
        } else if (new_mesh.mtl->diffuse_tex->data == NULL) {
            *new_mesh.mtl->diffuse_tex = tex;
        }
    } else {
        new_mesh.mtl = (material *)malloc(sizeof(material));
        new_mesh.mtl->ambient_color = (vec3){0.2f, 0.2f, 0.2f};
        new_mesh.mtl->diffuse_color = (vec3){1.0f, 1.0f, 1.0f};
        new_mesh.mtl->specular_color = (vec3){1.0f, 1.0f, 1.0f};
        new_mesh.mtl->shininess = 32.0f;
        new_mesh.mtl->transparency = 1.0f;
        new_mesh.mtl->illumination_model = 2;

        if (tex.data != NULL) {
            new_mesh.mtl->diffuse_tex = (texture *)malloc(sizeof(texture));
            *new_mesh.mtl->diffuse_tex = tex;
        } else {
            new_mesh.mtl->diffuse_tex = NULL;
        }
        new_mesh.mtl->specular_tex = NULL;
        new_mesh.mtl->normal_tex = NULL;
        
    }

    for (int i = 0; i < count; i++) {
        free(filepaths[i]);
    }
    
    free(filepaths);
    free(objpath);
    free(mtlpath);
    free(diffpath);


    return new_mesh;

}

/*
 * =============================================================================
 *                       OPTIONAL MENU IMPLEMENTATION
 * =============================================================================
 */

#ifdef C3D_MENU

#ifdef C3D_CUSTOM_PATHS  
#else 
#ifndef C3D_REL_SCENES_READ_PATH 
#define C3D_REL_SCENES_READ_PATH "./assets/scenes/*"
#endif
#ifndef C3D_REL_MODELS_READ_PATH 
#define C3D_REL_MODELS_READ_PATH "./assets/models/*"
#endif
#ifndef C3D_MODELS_READ_PATH     
#define C3D_MODELS_READ_PATH     "./assets/models"
#endif
#endif

STDC3DDEF void c3d_behavior_rotate(display *d, int argc, char **args) {
    if (argc < 4) return;
    const char *target = args[1];
    char axis = args[2][0];
    float angle = atof(args[3]);

    if (!strcmp(target, "ALL")) {
        for (int i = 0; i < d->mesh_count; i++) {
            mat4 mat;
            if (axis == 'X') mat = c3d_mat4rtx(C3D_DEG2RAD(angle));
            if (axis == 'Y') mat = c3d_mat4rty(C3D_DEG2RAD(angle));
            if (axis == 'Z') mat = c3d_mat4rtz(C3D_DEG2RAD(angle));
            c3d_meshrel(&d->meshes[i], mat);
        }
    } else {
        for (int i = 0; i < d->mesh_count; i++) {
            if (!strcmp(d->meshes[i].name, target)) {
                mat4 mat;
                if (axis == 'X') mat = c3d_mat4rtx(C3D_DEG2RAD(angle));
                if (axis == 'Y') mat = c3d_mat4rty(C3D_DEG2RAD(angle));
                if (axis == 'Z') mat = c3d_mat4rtz(C3D_DEG2RAD(angle));
                c3d_meshrel(&d->meshes[i], mat);
            }
        }
    }
}

STDC3DDEF void c3d_behavior_movetomesh(display *d, int argc, char **args) {
    if (argc < 4) return;
    const char *source_name = args[1];
    const char *target_name = args[2];
    float step = atof(args[3]);

    mesh *source = NULL;
    mesh *target = NULL;

    for (int i = 0; i < d->mesh_count; i++) {
        if (!strcmp(d->meshes[i].name, source_name)) {
            source = &d->meshes[i];
        } else if (!strcmp(d->meshes[i].name, target_name)) {
            target = &d->meshes[i];
        }
    }
    
    if (source && target) {
        vec3 source_center = c3d_meshcenter(*source);
        vec3 target_center = c3d_meshcenter(*target);
        vec3 direction = {target_center.x - source_center.x, target_center.y - source_center.y, target_center.z - source_center.z};
        c3d_vec3normalize(&direction);
        
        mat4 mat = c3d_mat4tra(direction.x * step, direction.y * step, direction.z * step);
        c3d_meshabs(source, mat);
    }
}

STDC3DDEF void c3d_behavior_moveto(display *d, int argc, char **args) {
    if (argc < 5) return;
    const char *target_name = args[1];
    float x = atof(args[2]);
    float y = atof(args[3]);
    float z = atof(args[4]);
    float step = atof(args[5]);

    mesh *target = NULL;

    for (int i = 0; i < d->mesh_count; i++) {
        if (!strcmp(d->meshes[i].name, target_name)) {
            target = &d->meshes[i];
            break;
        }
    }
    
    if (target) {
        vec3 target_position = (vec3){x, y, z};
        vec3 source_center = c3d_meshcenter(*target);
        vec3 direction = {target_position.x - source_center.x, target_position.y - source_center.y, target_position.z - source_center.z};
        c3d_vec3normalize(&direction);
        
        mat4 mat = c3d_mat4tra(direction.x * step, direction.y * step, direction.z * step);
        c3d_meshabs(target, mat);
    }
}

STDC3DDEF void c3d_behavior_swaptex(display *d, int argc, char **args) {
    if (argc < 3) return;
    const char *target_name = args[1];
    const char *new_texture_path = args[2];

    for (int i = 0; i < d->mesh_count; i++) {
        if (!strcmp(d->meshes[i].name, target_name)) {
            if (d->meshes[i].mtl->diffuse_tex) {
                free(d->meshes[i].mtl->diffuse_tex->data);
                free(d->meshes[i].mtl->diffuse_tex);
            }
            texture new_texture = c3d_loadimg(new_texture_path);
            d->meshes[i].mtl->diffuse_tex = malloc(sizeof(texture));
            *d->meshes[i].mtl->diffuse_tex = new_texture;
            break;
        }
    }
}

STDC3DDEF void c3d_behavior_swapmesh(display *d, int argc, char **args) {
    if (argc < 3) return;
    const char *old_mesh_name = args[1];
    const char *new_mesh_path = args[2];

    for (int i = 0; i < d->mesh_count; i++) {
        if (!strcmp(d->meshes[i].name, old_mesh_name)) {
            if (d->meshes[i].tris) {
                free(d->meshes[i].tris);
            }
            if (d->meshes[i].mtl->diffuse_tex) {
                free(d->meshes[i].mtl->diffuse_tex->data);
                free(d->meshes[i].mtl->diffuse_tex);
            }

            char *full_path = c3d_strcat3(C3D_MODELS_READ_PATH, "/", new_mesh_path);

            mesh new_mesh = c3d_loadmesh(full_path);
            c3d_meshadd(d, new_mesh);

            d->meshes[i] = new_mesh;
            break;
        }
    }
}

STDC3DDEF void c3d_behavior_rotate_id(display *d, int argc, char **args) {
    if (argc < 4) return;
    int id = atoi(args[1]);
    char axis = args[2][0];
    float angle = atof(args[3]);

    if (id >= 0 && id < d->mesh_count) {
        mat4 mat;
        if (axis == 'X') mat = c3d_mat4rtx(C3D_DEG2RAD(angle));
        if (axis == 'Y') mat = c3d_mat4rty(C3D_DEG2RAD(angle));
        if (axis == 'Z') mat = c3d_mat4rtz(C3D_DEG2RAD(angle));
        c3d_meshrel(&d->meshes[id], mat);
    }
}

STDC3DDEF void c3d_behavior_movetomesh_id(display *d, int argc, char **args) {
    if (argc < 4) return;
    int source_id = atoi(args[1]);
    int target_id = atoi(args[2]);
    float step = atof(args[3]);

    if (source_id >= 0 && source_id < d->mesh_count && target_id >= 0 && target_id < d->mesh_count) {
        vec3 source_center = c3d_meshcenter(d->meshes[source_id]);
        vec3 target_center = c3d_meshcenter(d->meshes[target_id]);
        vec3 direction = {target_center.x - source_center.x, target_center.y - source_center.y, target_center.z - source_center.z};
        c3d_vec3normalize(&direction);
        
        mat4 mat = c3d_mat4tra(direction.x * step, direction.y * step, direction.z * step);
        c3d_meshabs(&d->meshes[source_id], mat);
    }
}

STDC3DDEF void c3d_behavior_moveto_id(display *d, int argc, char **args) {
    if (argc < 5) return;
    int id = atoi(args[1]);
    float x = atof(args[2]);
    float y = atof(args[3]);
    float z = atof(args[4]);
    float step = atof(args[5]);

    if (id >= 0 && id < d->mesh_count) {
        vec3 target_position = (vec3){x, y, z};
        vec3 source_center = c3d_meshcenter(d->meshes[id]);
        vec3 direction = {target_position.x - source_center.x, target_position.y - source_center.y, target_position.z - source_center.z};
        c3d_vec3normalize(&direction);
        
        mat4 mat = c3d_mat4tra(direction.x * step, direction.y * step, direction.z * step);
        c3d_meshabs(&d->meshes[id], mat);
    }
}

STDC3DDEF void c3d_behavior_swaptex_id(display *d, int argc, char **args) {
    if (argc < 3) return;
    int id = atoi(args[1]);
    const char *new_texture_path = args[2];

    if (id >= 0 && id < d->mesh_count) {
        if (d->meshes[id].mtl->diffuse_tex) {
            free(d->meshes[id].mtl->diffuse_tex->data);
            free(d->meshes[id].mtl->diffuse_tex);
        }
        texture new_texture = c3d_loadimg(new_texture_path);
        d->meshes[id].mtl->diffuse_tex = malloc(sizeof(texture));
        *d->meshes[id].mtl->diffuse_tex = new_texture;
    }
}

STDC3DDEF void c3d_behavior_swapmesh_id(display *d, int argc, char **args) {
    if (argc < 3) return;
    int id = atoi(args[1]);
    const char *new_mesh_path = args[2];

    if (id >= 0 && id < d->mesh_count) {
        if (d->meshes[id].tris) free(d->meshes[id].tris);
        if (d->meshes[id].mtl->diffuse_tex) free(d->meshes[id].mtl->diffuse_tex->data), free(d->meshes[id].mtl->diffuse_tex);

        char *full_path = c3d_strcat3(C3D_REL_MODELS_READ_PATH, "/", new_mesh_path);

        mesh new_mesh = c3d_loadmesh(full_path);
        c3d_meshadd(d, new_mesh);
                
        d->meshes[id] = new_mesh;

        free(full_path);
    }
}

STDC3DDEF void c3d_behavior_loopmesh(display *d, int argc, char **args) {
    if (argc < 3) return;
    int id = atoi(args[1]);
    int frame_count = atoi(args[2]);

    int frame_index = 0;
    if (id >= 0 && id < d->mesh_count) {
        char frame_path[256];
        snprintf(frame_path, sizeof(frame_path), "assets/models/%s%d.obj", d->meshes[id].name, frame_index);
        frame_index = (frame_index + 1) % frame_count;

        if (d->meshes[id].tris) free(d->meshes[id].tris);
        if (d->meshes[id].mtl->diffuse_tex) free(d->meshes[id].mtl->diffuse_tex->data), free(d->meshes[id].mtl->diffuse_tex);
        d->meshes[id] = c3d_loadmesh(frame_path);
    }
}

STDC3DDEF void c3d_behavior_scalemesh(display *d, int argc, char **args) {
    if (argc < 4) return;
    int id = atoi(args[1]);
    float sx = atof(args[2]);
    float sy = atof(args[3]);
    float sz = atof(args[4]);

    if (id >= 0 && id < d->mesh_count) {
        mat4 mat = c3d_mat4scl(sx, sy, sz);
        c3d_meshrel(&d->meshes[id], mat);
    }
}

STDC3DDEF void c3d_behavior_colorize(display *d, int argc, char **args) {
    if (argc < 4) return;
    int id = atoi(args[1]);
    int r = atoi(args[2]);
    int g = atoi(args[3]);
    int b = atoi(args[4]);

    int width = 128;
    int height = 128;

    if (id >= 0 && id < d->mesh_count) {
        d->meshes[id].mtl->diffuse_tex = (c3d_intuc *)malloc(width * height * 3 * sizeof(c3d_intuc));

        for (int i = 0; i < width * height * 3; i += 3) {
            d->meshes[id].mtl->diffuse_tex->data[i] = (vec3){r, g, b};     
            d->meshes[id].mtl->diffuse_tex->data[i + 1] = (vec3){r, g, b}; 
            d->meshes[id].mtl->diffuse_tex->data[i + 2] = (vec3){r, g, b}; 
        }

        d->meshes[id].mtl->diffuse_tex->width = width;
        d->meshes[id].mtl->diffuse_tex->height = height;
        d->meshes[id].mtl->diffuse_tex->channels = 3; 
    }
}


/**
 * Loads scene from path into a display.
 */
STDC3DDEF void c3d_loadscene(display *d, char *path){
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    }
    
    c3d_resetdisplay(d);

    char buffer[50];
    char line[256];

    while (fgets(line, sizeof(line), f)){
        // char *comment = strchar(line, '#');
        // line[(int)(comment-line)] = '\0';

        if (line[0] == '['){
            sscanf_s(line, "[%49[^]]]", buffer, (unsigned)sizeof(buffer));
            continue;
        }

        if (!strcmp(buffer, "camera")){

            char key[50];
            float x, y, z, val;

            if (sscanf_s(line, "%49s %f %f %f", key, (unsigned)_countof(key), &x, &y, &z) == 4) {
                if (!strcmp(key, "position"))  { 
                    d->camera.pos.x = x; 
                    d->camera.pos.y = y;
                    d->camera.pos.z = z;
                }
            }
            
            if (sscanf_s(line, "%49s %f", key, (unsigned)_countof(key), &val) == 2){
                if (!strcmp(key, "fov"))   {d->camera.fov = val;}
                if (!strcmp(key, "speed")) {d->camera.speed = val;}
            }

        }
        if (!strcmp(buffer, "meshes")){

            char mpath[50];
            float x, y, z, scale_x, scale_y, scale_z;

            if (sscanf_s(line, "%49s %f %f %f %f %f %f", mpath, (unsigned)_countof(mpath), &x, &y, &z, &scale_x, &scale_y, &scale_z) == 7){

                char *full_path = c3d_strcat3(C3D_MODELS_READ_PATH, "/", mpath);
                
                mesh new_mesh = c3d_loadmesh(full_path);
                new_mesh.name = (char*)realloc(new_mesh.name, strlen(mpath) * sizeof(char));
                memcpy(new_mesh.name, mpath, strlen(mpath) + 1);
                
                c3d_meshadd(d, new_mesh);

                mat4 mtranslate = c3d_mat4tra(x, y, z);
                mat4 mscale = c3d_mat4scl(scale_x, scale_y, scale_z);
                mat4 mtransform = c3d_mat4mul(mtranslate, mscale);
                
                c3d_meshabs(&new_mesh, mtransform);

                c3d_meshadd(d, new_mesh);

                free(full_path);
            }
            
        }
        if (!strcmp(buffer, "display")){

            char key[50];
            int r, g, b;

            if (sscanf_s(line, "%49s %d %d %d", key, (unsigned)_countof(key), &r, &g, &b) == 4){
                if (!strcmp(key, "background_color")){ 
                    d->background_color.x = r; 
                    d->background_color.y = g; 
                    d->background_color.z = b; 
                }
            }
        }
        if (!strcmp(buffer, "lights")){
            
            float x, y, z, brightness, radius;
            int r, g, b;

            if (sscanf_s(line, "%f %f %f %d %d %d %f %f", &x, &y, &z, &r, &g, &b, &brightness, &radius) == 8){
                light new_light = (light){(vec3){x, y, z}, (vec3){r, g, b}, brightness, radius};
                c3d_lightadd(d, new_light);
            }
        }

        if (!strcmp(buffer, "continuous") || !strcmp(buffer, "startup")){
            behavior_type bt = !strcmp(buffer, "continuous") ? C3D_CONTINUOUS_BEHAVIOR : C3D_STARTUP_BEHAVIOR;
            char *line_cpy = _strdup(line);
            char *tokens[10];
            int tcount = 0;
            char *tok = strtok(line_cpy, " \t\r\n");
            while (tok && tcount < 10) {
                tokens[tcount++] = tok;
                tok = strtok(NULL, " \t\r\n");
            }

            if (tcount > 0) {
                if (!strcmp(tokens[0], "rotate") && tcount == 4)        c3d_behavioradd(d, c3d_behavior_rotate,         bt, tcount, tokens);
                if (!strcmp(tokens[0], "movetomesh") && tcount == 4)    c3d_behavioradd(d, c3d_behavior_movetomesh,     bt, tcount, tokens);
                if (!strcmp(tokens[0], "moveto") && tcount == 6)        c3d_behavioradd(d, c3d_behavior_moveto,         bt, tcount, tokens);
                if (!strcmp(tokens[0], "swaptex") && tcount == 3)       c3d_behavioradd(d, c3d_behavior_swaptex,        bt, tcount, tokens);
                if (!strcmp(tokens[0], "swapmesh") && tcount == 3)      c3d_behavioradd(d, c3d_behavior_swapmesh,       bt, tcount, tokens);
                if (!strcmp(tokens[0], "rotate_id") && tcount == 4)     c3d_behavioradd(d, c3d_behavior_rotate_id,      bt, tcount, tokens);
                if (!strcmp(tokens[0], "movetomesh_id") && tcount == 4) c3d_behavioradd(d, c3d_behavior_movetomesh_id,  bt, tcount, tokens);
                if (!strcmp(tokens[0], "moveto_id") && tcount == 6)     c3d_behavioradd(d, c3d_behavior_moveto_id,      bt,tcount, tokens);
                if (!strcmp(tokens[0], "swaptex_id") && tcount == 3)    c3d_behavioradd(d, c3d_behavior_swaptex_id,     bt, tcount, tokens);
                if (!strcmp(tokens[0], "swapmesh_id") && tcount == 3)   c3d_behavioradd(d, c3d_behavior_swapmesh_id,    bt, tcount, tokens);
                if (!strcmp(tokens[0], "loopmesh") && tcount == 3)      c3d_behavioradd(d, c3d_behavior_loopmesh,       bt, tcount, tokens);
                if (!strcmp(tokens[0], "scalemesh") && tcount == 4)     c3d_behavioradd(d, c3d_behavior_scalemesh,      bt, tcount, tokens);
                if (!strcmp(tokens[0], "colorize") && tcount == 4)      c3d_behavioradd(d, c3d_behavior_colorize,       bt, tcount, tokens);
            }

            free(line_cpy);
        }
    }

    fclose(f);
}

/**
 * Built-in menu for ease of interaction
 * 
 * Reads user input and returns the option. 
 */
void c3d_retgui(display *d){
    char **paths;
    char **folderpaths;
    int count;
    int foldercount;

    c3d_filelist(C3D_REL_SCENES_READ_PATH, &paths, &count);
    c3d_folderlist(C3D_REL_MODELS_READ_PATH, &folderpaths, &foldercount);

    char*menu =
    "\033[H"
    "╔═════════════════════════════════╗\n"
    "║                                 ║\n"
    "║     █████╗  ██████╗ ██████╗     ║\n"
    "║    ██╔══██╗ ╚════██╗██╔══██╗    ║\n"
    "║    ██║  ╚═╝  █████╔╝██║  ██║    ║\n"
    "║    ██║  ██╗  ╚═══██╗██║  ██║    ║\n"
    "║    ╚█████╔╝ ██████╔╝██████╔╝    ║\n"
    "║     ╚════╝ ╚═════╝ ╚═════╝      ║\n"
    "║                                 ║\n"
    "║    Use LEFT/RIGHT to switch,    ║\n"
    "║  UP/DOWN to navigate, ENTER to  ║\n"
    "║     select, ESCAPE to EXIT!     ║\n"
    "║                                 ║\n";

    int current = 0;
    int type = 0;

    Sleep(100);

    while(true){
        fprintf(stdout, menu);
        fprintf(stdout, (type == 0) ? "║      Reading scenes files       ║\n" : "║       Reading .OBJ files        ║\n");
        fprintf(stdout, "║                                 ║\n");

        int selected_bad_texture = 0;

        for (int i = 0; i < ((type == 0) ? count : foldercount); i++) {
            const char *foldername = (type == 0) ? paths[i] : folderpaths[i];
            int has_texture = (type == 1) ? c3d_hastexture(foldername) : 1;
            if (i == current) {
                if (!has_texture) {
                    fprintf(stdout, "║%s <X>\t%-20s\t⬤%s ║\n", C3D_ANSI_COLOR(255, 100, 100), foldername, C3D_ANSI_RESET_COLOR);
                    selected_bad_texture = 1;
                } else {
                    fprintf(stdout, "║%s <X>\t%-20s\t%s  ║\n", C3D_ANSI_COLOR(255, 255, 255), foldername, C3D_ANSI_RESET_COLOR);
                    selected_bad_texture = 0;
                }
            } else {
                if (!has_texture) {
                    fprintf(stdout, "║%s < >\t%-20s\t⬤%s ║\n", C3D_ANSI_COLOR(255, 100, 100), foldername, C3D_ANSI_RESET_COLOR);
                } else {
                    fprintf(stdout, "║ < >\t%-20s\t  ║\n", foldername);
                }
            }
        }
        fprintf(stdout, "╚═════════════════════════════════╝");

        if (selected_bad_texture) {
            fprintf(stdout, "\n%s⬤ No texture files found at specified path.%s", C3D_ANSI_COLOR(255, 100, 100), C3D_ANSI_RESET_COLOR);
        } else {
            fprintf(stdout, "\n\033[K");
        }

        if (GetAsyncKeyState(VK_LEFT) & C3D_KEY_PRESSED) {
            type = (type == 0) ? 1 : 0;
            current = 0;
            Sleep(150);
            C3D_SYS_ANSI_RESET;
        }
        if (GetAsyncKeyState(VK_RIGHT) & C3D_KEY_PRESSED) {
            type = (type == 1) ? 0 : 1;
            current = 0;
            Sleep(150);
            C3D_SYS_ANSI_RESET;
        }
        if (GetAsyncKeyState(VK_UP) &C3D_KEY_PRESSED) {
            current = (current == 0) ? ((type == 0) ? count: foldercount) - 1 : current - 1;
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_DOWN) &C3D_KEY_PRESSED) {
            current = (current == ((type == 0) ? count: foldercount)  - 1) ? 0 : current + 1;
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_RETURN) &C3D_KEY_PRESSED) {
            Sleep(100);

            if (type == 0){
                char full_path[256];
                snprintf(full_path, sizeof(full_path), "./assets/scenes/%s", paths[current]);
                C3D_SYS_ANSI_RESET;
                fprintf(stdout, "Contents of %s:\n\n", paths[current]);
                c3d_show_file_contents(full_path);

                fprintf(stdout, "\n\n|RETURN| Continue\t\t|ESCAPE| Back\n");

                while(1){
                    if (GetAsyncKeyState(VK_RETURN) &C3D_KEY_PRESSED){
                        C3D_SYS_ANSI_RESET;
                        c3d_loadscene(d, full_path);
                        return;
                    }
                    if (GetAsyncKeyState(VK_ESCAPE) &C3D_KEY_PRESSED){
                        C3D_SYS_ANSI_RESET;
                        Sleep(100);
                        break;
                    }
                }
            } else {
                C3D_SYS_ANSI_RESET;
                fprintf(stdout, "Loading Object from %s...\n", folderpaths[current]);
                c3d_loadobjfolder(d, folderpaths[current]);
                return;
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) &C3D_KEY_PRESSED){
            Sleep(100);
            break;
        }
    }
}

#endif

/*
 * =============================================================================
 *                WINDOWS.H STANDARD LIBRARY HELPER FUNCTIONS
 * =============================================================================
 */

/**
 * Concatenates 3 strings together.
 */
STDC3DDEF c3d_intuc *c3d_strcat3(const char *prefix, const char *string, const char *suffix){
    const size_t len_prefix = strlen(prefix);
    const size_t len_string = strlen(string);
    const size_t len_suffix = strlen(suffix);

    char *new = malloc(len_prefix + len_string + len_suffix + 1);

    if (new == NULL){
        fprintf(stderr, "Memory allocation failed at strcat of parameters %s, %s, %s.", prefix, string, suffix);
        exit(EXIT_FAILURE);
    };

    memcpy(new, prefix, len_prefix);
    memcpy(new + len_prefix, string, len_string);
    memcpy(new + len_prefix + len_string, suffix, len_suffix + 1);
    
    return new;
}

STDC3DDEF void c3d_show_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Unable to open file: %s\n", filename);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

STDC3DDEF bool c3d_hastexture(const char *folder) {
    char **filepaths; 
    int count;
    char *temp = c3d_strcat3(C3D_MODELS_READ_PATH, "/", folder);
    char *fulldir = c3d_strcat3(temp, "/", "*");
    
    c3d_filelist(fulldir, &filepaths, &count);
    
    char *png = ".PNG"; 
    char *jpg = ".JPG";

    int idx = 0, c = 0;
    while (idx < count){
        char *filepath = filepaths[idx];
        for (int i = 0; filepath[i] != '\0'; i++) {
            filepath[i] = toupper(filepath[i]);
        }

        if (strstr(filepath, png) != NULL || strstr(filepath, jpg) != NULL){
            c++;
            break;
        }

        idx++;
    }

    if (c > 0) {
        return true;
    }

    return false;
}

/**
 * Lists file in a directory
 */
STDC3DDEF void c3d_filelist(const char *path, char ***out, int *count) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No scenes found in the %s directory.\n", path);
        *out = NULL;
        *count = 0;
        return;
    }

    *count = 0;
    char **file_list = NULL;

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            file_list = realloc(file_list, (*count + 1) * sizeof(char *));
            if (!file_list) {
                fprintf(stderr, "Memory allocation failed in filelist.\n");
                exit(EXIT_FAILURE);
            }
            file_list[*count] = malloc(strlen(findFileData.cFileName) + 1);
            if (!file_list[*count]) {
                fprintf(stderr, "Memory allocation failed for file name.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(file_list[*count], findFileData.cFileName);
            (*count)++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    *out = file_list;
}

/**
 * Loads object from folder
 */
STDC3DDEF c3d_intui c3d_loadobjfolder(display *d, const char *folder) {
    c3d_resetdisplay(d);

    char *full_path = c3d_strcat3(C3D_MODELS_READ_PATH, "/", folder);
    mesh new_mesh = c3d_loadmesh(full_path);
    c3d_meshadd(d, new_mesh);
 
    free(full_path);
    return 1;
}

/**
 * Lists folders in a directory
 */
STDC3DDEF void c3d_folderlist(const char *path, char ***out, int *count) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No directories found in the %s path.\n", path);
        *out = NULL;
        *count = 0;
        return;
    }

    *count = 0;
    char **folder_list = NULL;

    do {
        if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
            strcmp(findFileData.cFileName, ".") != 0 && 
            strcmp(findFileData.cFileName, "..") != 0) {

            folder_list = realloc(folder_list, (*count + 1) * sizeof(char *));
            if (!folder_list) {
                fprintf(stderr, "Memory allocation failed in folderlist.\n");
                exit(EXIT_FAILURE);
            }
            folder_list[*count] = malloc(strlen(findFileData.cFileName) + 1);
            if (!folder_list[*count]) {
                fprintf(stderr, "Memory allocation failed for folder name.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(folder_list[*count], findFileData.cFileName);
            (*count)++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    *out = folder_list;
}

/** 
 * Creates a system call to fetch the console window's size 
*/
window c3d_winsize(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    window new_window;
    int ret;
    ret = GetConsoleScreenBufferInfo(GetStdHandle( STD_OUTPUT_HANDLE ),&csbi);
    if (ret){
        new_window.width = csbi.dwSize.X;
        new_window.height = csbi.dwSize.Y;
    }
    return new_window;
}

/** 
 * Initializes the Windows API 
*/
void c3d_wininit(window wprop){
    ShowCursor(FALSE);
    SetConsoleOutputCP(CP_UTF8);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT window_size = {0, 0, wprop.width, wprop.height};
    SetConsoleWindowInfo(hConsole, TRUE, &window_size);
    COORD buffer_size = {wprop.width, wprop.height};
    SetConsoleScreenBufferSize(hConsole, buffer_size);

    system("cls");
}

/**
 * Window auto resize 
 */
void c3d_auto_winres(display *d, cam *c){
    window size = c3d_winsize();
    int correction_factor = 5;
    d->display_width = size.width - correction_factor;
    d->display_height = size.height - correction_factor;
    c->aspect = (size.width - correction_factor) / (size.height - correction_factor);
}

/*
 * =============================================================================
 *                      MISCELLANEOUS HELPER FUNCTIONS
 * =============================================================================
 */

/**
 * Initializes a display.
 */
display c3d_initdisplay(cam camera, int display_width, int display_height, vec3 background_color){
    display new_display;
    new_display.running = true;
    new_display.meshes = NULL;
    new_display.behaviors = NULL;
    new_display.lights = NULL;
    new_display.light_count = 0;
    new_display.mesh_count = 0;
    new_display.behavior_count = 0;
    new_display.frame_count = 0;
    new_display.camera = camera;
    new_display.display_width = display_width;
    new_display.display_height = display_height;
    new_display.background_color = background_color;
    return new_display;
}

cam c3d_initcam(vec3 position, float fov, float speed){
    cam new_cam;
    new_cam.pos = position;
    new_cam.speed = speed;
    new_cam.fov = fov;
    new_cam.theta = 0.0f;
    new_cam.aspect = 1.0f;
    new_cam.fnear = 0.2f;
    new_cam.ffar = 500.0f;
    new_cam.yaw = 0.0f;
    new_cam.pitch = 0.0f;
    new_cam.matrot = (mat4){{0.0f}};
    return new_cam;
}

/**
 * Resets a display struct
 */
void c3d_resetdisplay(display *d) {
    d->background_color.x = 0;
    d->background_color.y = 0;
    d->background_color.z = 0;

    d->camera.pos.x = 0.0f;
    d->camera.pos.y = 0.0f;
    d->camera.pos.z = 0.0f;
    d->camera.fov = 70.0f;
    d->camera.speed = 0.5f;

    d->display_width = 800;
    d->display_height = 600;

    for (int i = 0; i < d->mesh_count; i++) {
        if (d->meshes[i].tris) {
            free(d->meshes[i].tris); 
        }
    }
    free(d->meshes); 
    d->meshes = NULL;

    // free(d->lights);
    // d->lights = NULL;  

    d->frame_count = 0;
    d->mesh_count = 0; 
}

/**
 * Adds a light to the display.
 */

void c3d_lightadd(display *d, light new_light){
    d->lights = (light *)realloc(d->lights, (d->light_count + 1) * sizeof(light));
    if (d->lights == NULL) {
        fprintf(stderr, "Memory allocation failed in c3d_lightadd.\n");
        exit(EXIT_FAILURE);
    }
    d->lights[d->light_count++] = new_light;
}

/**
 * Adds a mesh to the display.
 */
void c3d_meshadd(display *d, mesh new_mesh) {
    d->meshes = (mesh *)realloc(d->meshes, (d->mesh_count + 1) * sizeof(mesh));
    
    if (d->meshes == NULL) {
        fprintf(stderr, "Memory allocation failed in c3d_meshadd.\n");
        exit(EXIT_FAILURE);
    }

    d->meshes[d->mesh_count++] = new_mesh;
}

/**
 * Adds a behavior to the display.
 */
void c3d_behavioradd(display *d, void (*func)(display *, int, char **), behavior_type type, int argc, char **args) {
    behavior *temp = realloc(d->behaviors, (d->behavior_count + 1) * sizeof(behavior));
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation failed in c3d_behavioradd.\n");
        exit(EXIT_FAILURE);
    }
    d->behaviors = temp;

    behavior *b = &d->behaviors[d->behavior_count++];
    b->func = func;
    b->argc = argc;
    b->type = type;
    b->args = malloc(argc * sizeof(char *));
    if (b->args == NULL) {
        fprintf(stderr, "Memory allocation failed for behavior arguments.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < argc; i++) {
        b->args[i] = strdup(args[i]);
        if (b->args[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for behavior argument string.\n");
            exit(EXIT_FAILURE);
        }
    }
}

STDC3DDEF void c3d_meshabs(mesh *A, mat4 B){
    mat3 invTranspose = c3d_mat4invtranspose3(B);

    for (int i = 0; i < A->tri_count; i++) {
        tri *triangle = &A->tris[i];

        triangle->vx = c3d_mat4vec3(triangle->vx, B);
        triangle->vy = c3d_mat4vec3(triangle->vy, B);
        triangle->vz = c3d_mat4vec3(triangle->vz, B);

        vec3 transformed_normal_vx = {
            invTranspose.m[0][0]*triangle->nvx.x + invTranspose.m[0][1]*triangle->nvx.y + invTranspose.m[0][2]*triangle->nvx.z,
            invTranspose.m[1][0]*triangle->nvx.x + invTranspose.m[1][1]*triangle->nvx.y + invTranspose.m[1][2]*triangle->nvx.z,
            invTranspose.m[2][0]*triangle->nvx.x + invTranspose.m[2][1]*triangle->nvx.y + invTranspose.m[2][2]*triangle->nvx.z
        };

        vec3 transformed_normal_vy = {
            invTranspose.m[0][0]*triangle->nvy.x + invTranspose.m[0][1]*triangle->nvy.y + invTranspose.m[0][2]*triangle->nvy.z,
            invTranspose.m[1][0]*triangle->nvy.x + invTranspose.m[1][1]*triangle->nvy.y + invTranspose.m[1][2]*triangle->nvy.z,
            invTranspose.m[2][0]*triangle->nvy.x + invTranspose.m[2][1]*triangle->nvy.y + invTranspose.m[2][2]*triangle->nvy.z
        };

        vec3 transformed_normal_vz = {
            invTranspose.m[0][0]*triangle->nvz.x + invTranspose.m[0][1]*triangle->nvz.y + invTranspose.m[0][2]*triangle->nvz.z,
            invTranspose.m[1][0]*triangle->nvz.x + invTranspose.m[1][1]*triangle->nvz.y + invTranspose.m[1][2]*triangle->nvz.z,
            invTranspose.m[2][0]*triangle->nvz.x + invTranspose.m[2][1]*triangle->nvz.y + invTranspose.m[2][2]*triangle->nvz.z
        };

        c3d_vec3normalize(&transformed_normal_vx);
        c3d_vec3normalize(&transformed_normal_vy);
        c3d_vec3normalize(&transformed_normal_vz);

        triangle->nvx = transformed_normal_vx;
        triangle->nvy = transformed_normal_vy;
        triangle->nvz = transformed_normal_vz;
    }
}

STDC3DDEF void c3d_meshrel(mesh *A, mat4 B){
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    float V = A->tri_count * 3;

    for (int i = 0; i < A->tri_count; i++){
        tri triangle = A->tris[i];

        sum_x += triangle.vx.x + triangle.vy.x + triangle.vz.x;
        sum_y += triangle.vx.y + triangle.vy.y + triangle.vz.y;
        sum_z += triangle.vx.z + triangle.vy.z + triangle.vz.z;
    }

    vec3 cvec = {sum_x/V, sum_y/V, sum_z/V};

    mat4 translate_neg_center = c3d_mat4tra(-cvec.x, -cvec.y, -cvec.z);
    mat4 translate_pos_center = c3d_mat4tra(cvec.x, cvec.y, cvec.z);

    c3d_meshabs(A, translate_neg_center);
    c3d_meshabs(A, B);
    c3d_meshabs(A, translate_pos_center);
}

STDC3DDEF vec3 c3d_meshcenter(mesh A){
    int total_vertices = A.tri_count * 3;
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;

    for (int i = 0; i < A.tri_count; i++){
        tri* t = &A.tris[i];
        sum_x += (t->vx.x + t->vy.x + t->vz.x);
        sum_y += (t->vx.y + t->vy.y + t->vz.y);
        sum_z += (t->vx.z + t->vy.z + t->vz.z);
    }

    vec3 center_vec3;
    center_vec3.x = sum_x / total_vertices;
    center_vec3.y = sum_y / total_vertices;
    center_vec3.z = sum_z / total_vertices;
    return center_vec3;
}

float c3d_getavgfps(){
    bool inited = false;
    LARGE_INTEGER freq;
    LARGE_INTEGER lastCounter;
    double elapsed = 0.0;  
    float frameCount = 0.0f;
    float avgFPS = 0.0f;

    if (!inited) {
        QueryPerformanceFrequency(&freq);      
        QueryPerformanceCounter(&lastCounter); 
        inited = true;
    }

    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);

    double delta = (double)(current.QuadPart - lastCounter.QuadPart)
                 / (double)freq.QuadPart;

    lastCounter = current;

    elapsed    += delta;
    frameCount += 1.0f;

    if (elapsed >= 1.0) {
        avgFPS    = frameCount / (float)elapsed;  
        elapsed   = 0.0;
        frameCount = 0.0f;
    }

    return avgFPS;
}

#endif
/*
 * =============================================================================
 *                             EVENT HANDLERS
 * =============================================================================
 */

#ifdef C3D_EVENT_HANDLER

STDC3DDEF LARGE_INTEGER frequency;
STDC3DDEF LARGE_INTEGER lastTime;
STDC3DDEF bool initDone = false;

STDC3DDEF double c3d_getdeltatime() {
    if (!initDone) {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        initDone = true;
        return 0.0; 
    }

    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);

    double dt = (double)(current.QuadPart - lastTime.QuadPart) / (double)frequency.QuadPart;
    lastTime = current;
    return dt;
}

void c3d_k_handle(display *d) {
    double dt = c3d_getdeltatime();

    float speed = (float)dt * d->camera.speed;
    float rotationSpeed = (float)dt * 1.0f;

    float zoomf = 0.1f;

    if (GetAsyncKeyState('W') & C3D_KEY_PRESSED) {
        vec3 forward = { -d->camera.matrot.m[2][0], -d->camera.matrot.m[2][1], -d->camera.matrot.m[2][2] };
        c3d_vec3normalize(&forward);
        d->camera.pos.x += forward.x * speed;
        d->camera.pos.y += forward.y * speed;
        d->camera.pos.z += forward.z * speed;
    }
    if (GetAsyncKeyState('S') & C3D_KEY_PRESSED) {
        vec3 backward = { d->camera.matrot.m[2][0], d->camera.matrot.m[2][1], d->camera.matrot.m[2][2] };
        c3d_vec3normalize(&backward);
        d->camera.pos.x += backward.x * speed;
        d->camera.pos.y += backward.y * speed;
        d->camera.pos.z += backward.z * speed;
    }
    if (GetAsyncKeyState('A') & C3D_KEY_PRESSED) {
        vec3 left = { -d->camera.matrot.m[0][0], -d->camera.matrot.m[0][1], -d->camera.matrot.m[0][2] };
        c3d_vec3normalize(&left);
        d->camera.pos.x += left.x * speed;
        d->camera.pos.y += left.y * speed;
        d->camera.pos.z += left.z * speed;
    }
    if (GetAsyncKeyState('D') & C3D_KEY_PRESSED) {
        vec3 right = {d->camera.matrot.m[0][0], d->camera.matrot.m[0][1], d->camera.matrot.m[0][2] };
        c3d_vec3normalize(&right);
        d->camera.pos.x += right.x * speed;
        d->camera.pos.y += right.y * speed;
        d->camera.pos.z += right.z * speed;
    }
    if (GetAsyncKeyState(VK_SPACE) & C3D_KEY_PRESSED) {
        d->camera.pos.y += speed;
    }
    if (GetAsyncKeyState(VK_SHIFT) & C3D_KEY_PRESSED) {
        d->camera.pos.y -= speed;
    }
    if (GetAsyncKeyState(VK_LEFT) & C3D_KEY_PRESSED) {
        d->camera.yaw += rotationSpeed;
    }
    if (GetAsyncKeyState(VK_RIGHT) & C3D_KEY_PRESSED) {
        d->camera.yaw -= rotationSpeed;
    }
    if (GetAsyncKeyState(VK_UP) & C3D_KEY_PRESSED) {
        d->camera.pitch += rotationSpeed;
    }
    if (GetAsyncKeyState(VK_DOWN) & C3D_KEY_PRESSED) {
        d->camera.pitch -= rotationSpeed;
    }
    if (GetAsyncKeyState('I') & C3D_KEY_PRESSED) {
        d->camera.speed += zoomf;
    }
    if (GetAsyncKeyState('O') & C3D_KEY_PRESSED) {
        d->camera.speed -= (d->camera.speed >= zoomf) ? zoomf : 0;
    }
    
    if (GetAsyncKeyState(VK_RETURN) & C3D_KEY_PRESSED || GetAsyncKeyState(VK_LBUTTON) & C3D_KEY_PRESSED) {
        light new_light = (light){(vec3){d->camera.pos.x, d->camera.pos.y, d->camera.pos.z}, (vec3){rand() % 256 /255.0f, rand() % 256 /255.0f, rand() % 256 /255.0f}, 1.0f, 0.5f};
        c3d_lightadd(d, new_light);
    }
}

void c3d_m_handle(display *d, POINT p0){
    POINT p1;
    if (GetCursorPos(&p1)){
        d->camera.yaw += (-p1.x + p0.x) * C3D_MOUSE_DELTA_SENSITIVITY;
        d->camera.pitch += (-p1.y + p0.y) * C3D_MOUSE_DELTA_SENSITIVITY;
        d->camera.pitch = C3D_CLAMP(d->camera.pitch, -1.5f, 1.5f);

    }

    mat4 pitch = c3d_mat4rtx(d->camera.pitch);
    mat4 yaw = c3d_mat4rty(d->camera.yaw);

    d->camera.matrot = c3d_mat4mul(pitch, yaw);

    // SetCursorPos(900, 500);
}

#endif

/*
 * =============================================================================
 *                             PHYSICS AND COLLISIONS
 * =============================================================================
 */

#ifdef __cplusplus
// c++ stuff
#endif

#endif /* C3D_IMPLEMENTATION */

#endif /* C3D_H */



/**
 * -
 * /MIT LICENSE
 * -
 * 
 * Copyright (c) 2024 Lucca Fabricio Magalhães
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

