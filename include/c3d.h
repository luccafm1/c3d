/* c3d.h
*
*
* 
* C3D is a 3d .obj file renderer written on pure Windows terminal.
* 
* 
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
*
*/




#include <stdio.h>   
#include <stdlib.h>  
#include <stdint.h>
#include <math.h>      
// #include <termios.h>    // Terminal manipulation.
// #include <unistd.h>     // Terminal manipulation.
#include <conio.h>      // Read keyboard events.
// #include <pthread.h>    // Multithreading and asynchronous progrmaming.
#include <windows.h>    // Windows console API. 
#include <stdbool.h> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Image processing.




/* Screen and display macros */
#define SCREEN_WIDTH 150
#define SCREEN_HEIGHT 150
#define RENDERING_SPEED 1e-10    /* Constant time interval for rendering, arbitrarily set as 1.0 * 10^10. */
#define PI 3.141592653589793238462643383279502884197
/* Linalg macros */
/**
 * Initializes a 3D vector.
 */
#define VEC3(x, y, z) ((vec3){x, y, z})
/**
 * Initializes a 2D vector.
 */
#define VEC2(x, y) ((vec2){x, y})
/**
 * Simple camera constructor. Omits some detail from the original and initializes some pre-configured camera details.
 */
#define CAM(position, fov, speed) (cam){position, {{0.0f}}, 0, fov, 1, 0.2f, 500.0f, 0.0f, 0.0f, speed}
/* Algebra macros */
#define DEG2RAD(x) ((x) * PI/180)
/* Object macros */
#define MAX_OBJ_FILE_LINE_COUNT 100000
#define _OBJ_IMG_FILE (const char*)"/diffuse.png"
#define _OBJ_FILE (const char*)"/main.obj"
#define INITIAL_VERTICES_CAPACITY   100000000
#define INITIAL_TEXCOORDS_CAPACITY  10000000
#define INITIAL_NORMALS_CAPACITY    100000000
#define INITIAL_FACES_CAPACITY      10000000
/* Misc macros */
#define MIN3(a, b, c) (min(min(a, b), c))
#define MAX3(a, b, c) (max(max(a, b), c))
#define SYS_ANSI_RESET system("cls"); char*ansi_ec="\033[H"; fwrite(ansi_ec, sizeof(char*), (sizeof(ansi_ec)/sizeof(ansi_ec[0])), stdout); fflush(stdout)
#define CLAMP(var, x, y) (var > y) ? y : ((var < x) ? x : var)
#define MOUSE_DELTA_SENSITIVITY 0.01f
#define KEY_PRESSED 0x8000
#define ACOLOR(R, G, B) "\033[38;2;" #R ";" #G ";" #B "m"
#define BGCOLOR(R, G, B) "\033[48;2;" #R ";" #G ";" #B "m"
#define RESET_COLOR "\033[0m"




/* Global variables for screen buffer */
CHAR_INFO screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
HANDLE hConsole;
SMALL_RECT consoleWriteArea = {0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1};





/**
 * A 2D vector, like a 3d vector, is a vector in space. In this case, it is defined by 2 points (x, y).
 * We typically use this to perform projections into 2D space.
 */
typedef struct vec2_t{
    float x, y;
} vec2;

/**
 * A 3D vector is a vector in space defined by 3 points (x, y, z).
 * We typically use this to perform transformations in 3D spaces.
 */
typedef struct vec3_t{
    float x, y, z;
} vec3;

/**
 * A triangle is a polygon defined by 3 vectors connected in space.
 * We also include its uvmap coordinates and normals for texture
 * mapping and shading respectively.
 */
typedef struct tri_t{
    vec3 vx, vy, vz;
    vec2 uvx, uvy, uvz;
    vec3 nvx, nvy, nvz;
} tri;

/**
 * Texture structure.
 * @param data :- an array containing the RGB channels of the image.
 * @param width :- the width of the image.
 * @param height :- the height of the image.
 * @param channels :- the image channels.
 */
typedef struct texture_t{
    vec3 *data;
    int width;
    int height;
    int channels;
} texture;

/**
 * A mesh is an array of triangles.
 * We also include its provided texture.
 */
typedef struct mesh_t{
    tri *tris;
    int tri_count;
    texture *tex;
} mesh;

/**
 * Camera structure.
 * @param pos the vector position of the camera
 * @param matrot the rotation matrix of the camera
 * @param theta the camera's rotation angle
 * @param fov the field of view
 * @param aspect the aspect ratio of the camera frustrum
 * @param fnear the camera frustrum's near position
 * @param ffar the camera frustrum's far position
 * @param yaw the camera's relative Y rotation, mostly for mouse communication
 * @param pitch like yaw, but for the relative X rotation. also for mouse communication
 * @param speed the camera speed
 */
typedef struct cam_t {
    vec3 pos;
    float matrot[4][4];
    float theta;
    float fov;
    float aspect;
    float fnear;
    float ffar;
    float yaw;
    float pitch;
    float speed;
} cam;

/**
 * Behavior type.
 */
typedef void (*behavior)(int argc, char **args);

/**
 * Display structure.
 * @param meshes an array containing all meshes to be rendered.
 * @param background a 3D vector representing RGB channels of the display's background color
 * @param mesh_count the size of the meshes array
 * @param camera a regular field of view camera.
 * @param display_width self-explanatory.
 * @param display_height self-explanatory.
 * @param frame_count counts the current frame in relation to the previous ones.
 * @param behaviors an array containing behaviors for external interaction with the display.
 */
typedef struct display_t {
    mesh* meshes;
    vec3 background;
    cam camera;
    behavior *behaviors;
    int display_width;
    int display_height;
    int behavior_count;
    int frame_count;
    int mesh_count;
} display;

/**
 * Window structure.
 * For the Windows API.
 */
typedef struct window_t{
    int width;
    int height;
} window;

/* Function prototypes */
void matprj(float fnear, float ffar, float fov_deg, float aspect, float proj[4][4]);
void matrtx(float theta, float mat[4][4]);
void matrty(float theta, float mat[4][4]);
void matmul(float A[4][4], float B[4][4], float C[4][4]);
vec3 veccross(vec3 a, vec3 b);
vec3 matvec(vec3 A, float B[4][4]);
void mattra(float x, float y, float z, float mat[4][4]);
vec2 project(vec3 vec, int screen_width, int screen_height);
void bresenham(display *d, wchar_t **buffer, vec2 v0, vec2 v1, wchar_t tex);
bool backface(tri t, vec3 pos);
void update(display* d);
void refresh(display *d, wchar_t **buffer, COLORREF **colorBuffer);
cam ccam(vec3 pos, float theta, float fov, float aspect, float fnear, float ffar, float speed);
display cdisp(cam camera, int display_width, int display_height, vec3 color);
void mshadd(display *d, mesh new_mesh);

/**
 * I used the ANSI reference sheet from the following sources:
 * 
 * https://ibr.ansi.org
 * https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
 */

// void refresh(display *d, wchar_t **buffer, COLORREF **colorBuffer) {
//     COORD bufferSize = { (SHORT)d->display_width, (SHORT)d->display_height };
//     COORD bufferCoord = { 0, 0 };
//     SMALL_RECT writeRegion = { 0, 0, (SHORT)(d->display_width - 1), (SHORT)(d->display_height - 1) };

//     CHAR_INFO *charBuffer = (CHAR_INFO *)malloc(sizeof(CHAR_INFO) * d->display_width * d->display_height);
//     for (int y = 0; y < d->display_height; y++) {
//         for (int x = 0; x < d->display_width; x++) {
//             int index = x + y * d->display_width;
//             charBuffer[index].Char.UnicodeChar = buffer[y][x];
//             charBuffer[index].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED; // White color
//         }
//     }

//     WriteConsoleOutputW(
//         hConsole,
//         charBuffer,
//         bufferSize,
//         bufferCoord,
//         &writeRegion
//     );

//     free(charBuffer);
// }

/**
 * Function for coloring a line on the terminal.
 * 
 * This function uses the ANSI escape character \033[ and defines
 * 38;2 for coloring on the screen, and the last three digits,
 * separated by the semicolons, are the R, G and B
 */ 
void refresh(display *d, wchar_t **buffer, COLORREF **colorBuffer) {
    static wchar_t *output_buffer = NULL;
    static size_t output_buffer_size = 0;

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

    int bgr = (int)(d->background.x);
    int bgg = (int)(d->background.y);
    int bgb = (int)(d->background.z);
    
    bgr = CLAMP(bgr, 0, 255);
    bgg = CLAMP(bgg, 0, 255);
    bgb = CLAMP(bgb, 0, 255);
    
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

void vec3normalize(vec3 *vec){
    float V = sqrt(pow(vec->x,2) + pow(vec->y,2) + pow(vec->z, 2));
    if (V != 0){
        vec->x /= V;
        vec->y /= V;
        vec->z /= V;
    }
}

/**
 * Fetch the center of a mesh
 */
vec3 mshcenter(mesh msh){
    int len = sizeof(msh.tris) / sizeof(msh.tris[0]) * 3;
    int sum_x = 0; int sum_y = 0; int sum_z = 0;
    for (int i = 0; i < len; i++){
        sum_x += msh.tris[i].vx.x + msh.tris[i].vy.x + msh.tris[i].vz.x;
        sum_y += msh.tris[i].vx.y + msh.tris[i].vy.y + msh.tris[i].vz.y;
        sum_z += msh.tris[i].vx.z + msh.tris[i].vy.z + msh.tris[i].vz.z;
    }
    vec3 center_vec3;
    center_vec3.x = sum_x/len;
    center_vec3.y = sum_y/len;
    center_vec3.z = sum_z/len;
    return center_vec3;
}

/**
 * Transform mesh triangles relative to absolute axes
 */
void mshabs(mesh *msh, float mat[4][4]){
    for (int i = 0; i < msh->tri_count; i++){
        tri triangle = msh->tris[i];

        msh->tris[i].vx = matvec(triangle.vx, mat);
        msh->tris[i].vy = matvec(triangle.vy, mat);
        msh->tris[i].vz = matvec(triangle.vz, mat);
    }
}

/**
 * Transform mesh triangles relative to its center
 */
void mshrel(mesh *msh, float mat[4][4]){
    float sum_x, sum_y, sum_z;
    float V = msh->tri_count * 3;

    for (int i = 0; i < msh->tri_count; i++){
        tri triangle = msh->tris[i];

        sum_x += triangle.vx.x + triangle.vy.x + triangle.vz.x;
        sum_y += triangle.vx.y + triangle.vy.y + triangle.vz.y;
        sum_z += triangle.vx.z + triangle.vy.z + triangle.vz.z;
    }

    vec3 cvec = VEC3(sum_x/V, sum_y/V, sum_z/V);

    float translate_neg_center[4][4];
    mattra(-cvec.x, -cvec.y, -cvec.z, translate_neg_center);

    float translate_pos_center[4][4];
    mattra(cvec.x, cvec.y, cvec.z, translate_pos_center);

    mshabs(msh, translate_neg_center);
    mshabs(msh, mat);
    mshabs(msh, translate_pos_center);
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
void matprj(float fnear, float ffar, float fov, float aspect, float proj[4][4]) {
    float f = 1.0f / tanf(0.5f * DEG2RAD(fov));
    proj[0][0] = f / aspect; proj[0][1] = 0.0f; proj[0][2] = 0.0f;                   proj[0][3] = 0.0f;
    proj[1][0] = 0.0f;       proj[1][1] = f;    proj[1][2] = 0.0f;                   proj[1][3] = 0.0f;
    proj[2][0] = 0.0f;       proj[2][1] = 0.0f; proj[2][2] = (ffar + fnear) / (fnear - ffar); proj[2][3] = (2.0f * ffar * fnear) / (fnear - ffar);
    proj[3][0] = 0.0f;       proj[3][1] = 0.0f; proj[3][2] = -1.0f;                  proj[3][3] = 0.0f;
}

/**
 * The following rotation matrices are direct reference from Wikipedia's article on rotation matrices:
 * 
 * https://en.wikipedia.org/wiki/3D_projection
 */

/**
 * Constructs rotation matrix around the X-axis.
 */
void matrtx(float theta, float mat[4][4]) {
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat[0][0] = 1.0f; mat[0][1] = 0.0f;        mat[0][2] = 0.0f;         mat[0][3] = 0.0f;
    mat[1][0] = 0.0f; mat[1][1] = cos_theta;   mat[1][2] = -sin_theta;   mat[1][3] = 0.0f;
    mat[2][0] = 0.0f; mat[2][1] = sin_theta;   mat[2][2] = cos_theta;    mat[2][3] = 0.0f;
    mat[3][0] = 0.0f; mat[3][1] = 0.0f;        mat[3][2] = 0.0f;         mat[3][3] = 1.0f;
}

/**
 * Constructs rotation matrix around the Y-axis.
 */
void matrty(float theta, float mat[4][4]) {
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat[0][0] = cos_theta;    mat[0][1] = 0.0f; mat[0][2] = sin_theta;   mat[0][3] = 0.0f;
    mat[1][0] = 0.0f;         mat[1][1] = 1.0f; mat[1][2] = 0.0f;        mat[1][3] = 0.0f;
    mat[2][0] = -sin_theta;   mat[2][1] = 0.0f; mat[2][2] = cos_theta;   mat[2][3] = 0.0f;
    mat[3][0] = 0.0f;         mat[3][1] = 0.0f; mat[3][2] = 0.0f;        mat[3][3] = 1.0f;
}

/**
 * Constructs rotation matrix around the Z-axis.
 */
void matrtz(float theta, float mat[4][4]) {
    float cos_theta = cosf(-theta);
    float sin_theta = sinf(-theta);
    mat[0][0] = cos_theta;    mat[0][1] = -sin_theta;  mat[0][2] = 0.0f;  mat[0][3] = 0.0f;
    mat[1][0] = sin_theta;    mat[1][1] = cos_theta;   mat[1][2] = 0.0f;  mat[1][3] = 0.0f;
    mat[2][0] = 0.0f;         mat[2][1] = 0.0f;        mat[2][2] = 1.0f;  mat[2][3] = 0.0f;
    mat[3][0] = 0.0f;         mat[3][1] = 0.0f;        mat[3][2] = 0.0f;  mat[3][3] = 1.0f;
    
}

/**
 * Multiplies two 4x4 matrices A and B, stores result in C.
 * 
 * https://spatial-lang.org/gemm
 */
void matmul(float A[4][4], float B[4][4], float C[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            C[i][j] = A[i][0]*B[0][j] + A[i][1]*B[1][j] + A[i][2]*B[2][j] + A[i][3]*B[3][j];
        }
    }
}

/**
 * Calculates the cross product of vectors a and b.
 */
vec3 veccross(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

/**
 * Multiplies vector A by matrix B.
 */
vec3 matvec(vec3 A, float B[4][4]) {
    float x = A.x, y = A.y, z = A.z;
    float vec1[4];
    vec1[0] = B[0][0]*x + B[0][1]*y + B[0][2]*z + B[0][3];
    vec1[1] = B[1][0]*x + B[1][1]*y + B[1][2]*z + B[1][3];
    vec1[2] = B[2][0]*x + B[2][1]*y + B[2][2]*z + B[2][3];
    vec1[3] = B[3][0]*x + B[3][1]*y + B[3][2]*z + B[3][3];
    vec3 result;
    result.x = vec1[0];
    result.y = vec1[1];
    result.z = vec1[2];
    return result;
}

float vecdot(vec3 A, vec3 B){
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

/* Translation matrix */
void mattra(float x, float y, float z, float mat[4][4]) {
    mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = x;
    mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = y;
    mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = z;
    mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f;
}

/* Scaling matrix */
void matscl(float x, float y, float z, float mat[4][4]) {
    mat[0][0] = x;    mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
    mat[1][0] = 0.0f; mat[1][1] = y;    mat[1][2] = 0.0f; mat[1][3] = 0.0f;
    mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = z;    mat[2][3] = 0.0f;
    mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f;
}

/**
 * Projects a 3d vector into a 2d one.
 */
vec2 project(vec3 vec, int screen_width, int screen_height) {
    vec2 proj;
    if (vec.z == 0) {
        proj.x = 0;
        proj.y = 0;
    } else {
        proj.x = vec.x / vec.z;
        proj.y = vec.y / vec.z;
    }

    vec2 screen;
    screen.x = (proj.x + 1.0f) * 0.5f * screen_width;
    screen.y = (1.0f - (proj.y + 1.0f) * 0.5f) * screen_height;

    return screen;
}


/**
 * Bresenham's algorithm. Draws a line on any given display.
 */
void bresenham(display *d, wchar_t **buffer, vec2 v0, vec2 v1, wchar_t tex){

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
            buffer[y1][x1] = tex;
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
 * Samples a texture
 */
vec3 texsample(texture *tex, float u, float v) {
    if (tex->data == NULL) {
        vec3 default_color = {1.0f, 1.0f, 1.0f};
        return default_color;
    }

    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    int tex_x = (int)(u * (tex->width - 1));
    int tex_y = (int)((1.0f - v) * (tex->height - 1)); 

    int index = tex_y * tex->width + tex_x;

    return tex->data[index];
}

/**
 * Edge function for rasterization
 */
float edge(vec2 v0, vec2 v1, vec2 v2){
    return (((v2.x-v0.x)*(v1.y-v0.y)) - ((v1.x-v0.x)*(v2.y-v0.y)));
}

/**
 * Algorithm to fill triangles on any given display.
 */
void rasterize(display *d, wchar_t **buffer, COLORREF **colorBuffer, float **depthBuffer, vec3 v0, vec3 v1, vec3 v2, tri t, texture *tex) {
    vec2 pv0 = project(v0, d->display_width, d->display_height);
    vec2 pv1 = project(v1, d->display_width, d->display_height);
    vec2 pv2 = project(v2, d->display_width, d->display_height);

    int minx = max(MIN3(pv0.x, pv1.x, pv2.x), 0);
    int maxx = min(MAX3(pv0.x, pv1.x, pv2.x), d->display_width - 1);
    int miny = max(MIN3(pv0.y, pv1.y, pv2.y), 0);
    int maxy = min(MAX3(pv0.y, pv1.y, pv2.y), d->display_height - 1);

    float area = edge(pv0, pv1, pv2);

    if (area == 0) {
        return;
    }

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            vec2 vxy = VEC2((float)x + 0.5f, (float)y + 0.5f);

            float w0 = edge(pv1, pv2, vxy) / area;
            float w1 = edge(pv2, pv0, vxy) / area;
            float w2 = edge(pv0, pv1, vxy) / area;
            
            float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f && z < depthBuffer[y][x]) {
                depthBuffer[y][x] = z;

                float u = w0 * t.uvx.x + w1 * t.uvy.x + w2 * t.uvz.x;
                float v = w0 * t.uvx.y + w1 * t.uvy.y + w2 * t.uvz.y;

                vec3 scolor = texsample(tex, u, v);

                COLORREF color = RGB(
                    (int)(scolor.x * 255.0f),
                    (int)(scolor.y * 255.0f),
                    (int)(scolor.z * 255.0f)
                );

                buffer[y][x] = L'█';
                colorBuffer[y][x] = color;
            }
        }
    }
}


/**
 * Calculates whether a vector is in view.
 * 
 * We will be discarding all triangles where the dot product of their surface normal and the 
 * camera-to-triangle vector is greater than or equal to zero:
 */
bool backface(tri t, vec3 pos){
    vec3 u; vec3 v;
    u.x = t.vy.x - t.vx.x;
    u.y = t.vy.y - t.vx.y;
    u.z = t.vy.z - t.vx.z;

    v.x = t.vz.x - t.vx.x;
    v.y = t.vz.y - t.vx.y;
    v.z = t.vz.z - t.vx.z;

    vec3 n = veccross(u, v);
    vec3 view = VEC3(t.vx.x - pos.x, t.vx.y - pos.y, t.vx.z - pos.z);
    
    float dot = vecdot(n, view);

    return (dot >= 0.0f);
}

/* The update loop. */
void update(display* d){

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

    float matProj[4][4];
    matprj(d->camera.fnear, d->camera.ffar, d->camera.fov, d->camera.aspect, matProj);

    float camTranslate[4][4];
    mattra(-d->camera.pos.x, -d->camera.pos.y, -d->camera.pos.z, camTranslate);

    float camRot[4][4];
    memcpy(camRot, d->camera.matrot, sizeof(float) * 16);

    float camView[4][4];
    matmul(camRot, camTranslate, camView);

    float matCam[4][4];
    matmul(matProj, camView, matCam);

    for (int i = 0; i < d->mesh_count; i++){
        mesh* m = &d->meshes[i];
        for (int j = 0; j < m->tri_count; j++){
            tri t = m->tris[j];

            vec3 v0_transformed = matvec(t.vx, matCam);
            vec3 v1_transformed = matvec(t.vy, matCam);
            vec3 v2_transformed = matvec(t.vz, matCam);

            if (backface(t, d->camera.pos) || v0_transformed.z <= 0 || v1_transformed.z <= 0 || v2_transformed.z <= 0 ){
                continue;
            }
            
            rasterize(d, buffer, colorBuffer, depthBuffer, v0_transformed, v1_transformed, v2_transformed, t, m->tex);
        }
    }

    d->frame_count++;
    refresh(d, buffer, colorBuffer);
    for (int i = 0; i < d->display_height; i++) {
        free(buffer[i]);
        free(colorBuffer[i]);
        free(depthBuffer[i]);
    }
    free(buffer);
    free(colorBuffer);
    free(depthBuffer);
}

/**
 * Loads an image, and transforms it into a texture.
 * 
 * Uses stb_image library
 */
texture loadimg(const char *path){
    texture tex;
    int w, h, comp;
    char *img = stbi_load(path, &w, &h, &comp, 3);

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
        tex.data[i].x = img[i * 3 + 0] / 255.0f; 
        tex.data[i].y = img[i * 3 + 1] / 255.0f;
        tex.data[i].z = img[i * 3 + 2] / 255.0f;
    }
    
    stbi_image_free(img);
    return tex;
}




typedef struct {
    vec3 *data;
    size_t size;
    size_t capacity;
} Vec3Array;

typedef struct {
    vec2 *data;
    size_t size;
    size_t capacity;
} Vec2Array;

typedef struct {
    tri *data;
    size_t size;
    size_t capacity;
} TriArray;

void initVec3Array(Vec3Array *array, size_t initial_capacity) {
    array->data = (vec3 *)malloc(initial_capacity * sizeof(vec3));
    if (!array->data) {
        perror("Failed to allocate memory for vertices.");
        exit(EXIT_FAILURE);
    }
    array->size = 0;
    array->capacity = initial_capacity;
}

void initVec2Array(Vec2Array *array, size_t initial_capacity) {
    array->data = (vec2 *)malloc(initial_capacity * sizeof(vec2));
    if (!array->data) {
        perror("Failed to allocate memory for texture coordinates.");
        exit(EXIT_FAILURE);
    }
    array->size = 0;
    array->capacity = initial_capacity;
}

void initTriArray(TriArray *array, size_t initial_capacity) {
    array->data = (tri *)malloc(initial_capacity * sizeof(tri));
    if (!array->data) {
        perror("Failed to allocate memory for faces.");
        exit(EXIT_FAILURE);
    }
    array->size = 0;
    array->capacity = initial_capacity;
}

void appendVec3(Vec3Array *array, vec3 value) {
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        vec3 *temp = (vec3 *)realloc(array->data, new_capacity * sizeof(vec3));
        if (!temp) {
            fprintf(stderr, "Failed to realloc memory for vertices. Requested capacity: %zu\n", new_capacity);
            free(array->data);
            exit(EXIT_FAILURE);
        }
        array->data = temp;
        array->capacity = new_capacity;
    }
    array->data[array->size++] = value;
}

void appendVec2(Vec2Array *array, vec2 value) {
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        vec2 *temp = (vec2 *)realloc(array->data, new_capacity * sizeof(vec2));
        if (!temp) {
            fprintf(stderr, "Failed to realloc memory for texture coordinates. Requested capacity: %zu\n", new_capacity);
            free(array->data);
            exit(EXIT_FAILURE);
        }
        array->data = temp;
        array->capacity = new_capacity;
    }
    array->data[array->size++] = value;
}

void appendTri(TriArray *array, tri value) {
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        tri *temp = (tri *)realloc(array->data, new_capacity * sizeof(tri));
        if (!temp) {
            fprintf(stderr, "Failed to realloc memory for faces. Requested capacity: %zu\n", new_capacity);
            free(array->data);
            exit(EXIT_FAILURE);
        }
        array->data = temp;
        array->capacity = new_capacity;
    }
    array->data[array->size++] = value;
}

/**
 * Parses a single face vertex and extracts vertex, texture, and normal indices.
 * Supports both 'v//n' and 'v/t/n' formats.
 *
 * @param token The face vertex token (e.g., "81673//248210" or "81673/1/248210").
 * @param vertex_index Pointer to store the vertex index.
 * @param texcoord_index Pointer to store the texture coordinate index (0 if not present).
 * @param normal_index Pointer to store the normal index (0 if not present).
 * @return true if parsing was successful, false otherwise.
 */
bool parseFaceVertex(const char *token, int *vertex_index, int *texcoord_index, int *normal_index) {
    int slash_count = 0;
    const char *ptr = token;
    while (*ptr) {
        if (*ptr == '/') slash_count++;
        ptr++;
    }

    if (slash_count == 0) {
        // Format: v
        if (sscanf(token, "%d", vertex_index) != 1) return false;
        *texcoord_index = 0;
        *normal_index = 0;
    }
    else if (slash_count == 1) {
        // Format: v/t
        if (sscanf(token, "%d/%d", vertex_index, texcoord_index) != 2) return false;
        *normal_index = 0;
    }
    else if (slash_count == 2) {
        if (strstr(token, "//")) {
            // Format: v//n
            if (sscanf(token, "%d//%d", vertex_index, normal_index) != 2) return false;
            *texcoord_index = 0;
        }
        else {
            // Format: v/t/n
            if (sscanf(token, "%d/%d/%d", vertex_index, texcoord_index, normal_index) != 3) return false;
        }
    }
    else {
        return false;
    }

    return true;
}

/**
 * Loads an obj file.
 * Reads the OBJ file once and parses all data in a single pass.
 */
mesh loadobj(const char *path, const char *imgpath) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror("No file found at specified path.");
        exit(EXIT_FAILURE);
    }

    char imgpath_jpg[256];
    strncpy(imgpath_jpg, imgpath, sizeof(imgpath_jpg) - 1);
    imgpath_jpg[sizeof(imgpath_jpg) - 1] = '\0';

    size_t len = strlen(imgpath_jpg);
    if (len > 3) {
        imgpath_jpg[len - 3] = 'j';
        imgpath_jpg[len - 2] = 'p';
        imgpath_jpg[len - 1] = 'g';
    }

    FILE *fimg = fopen(imgpath, "r");
    if (!fimg) {
        fimg = fopen(imgpath_jpg, "r"); 
        if (fimg) {
            imgpath = imgpath_jpg;
        }
    }
    if (!fimg) {
        perror("No image file found at specified path.");
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(fimg); 
    
    Vec3Array vertices;
    Vec2Array texcoords;
    Vec3Array normals;
    TriArray faces;

    initVec3Array(&vertices, INITIAL_VERTICES_CAPACITY);
    initVec2Array(&texcoords, INITIAL_TEXCOORDS_CAPACITY);
    initVec3Array(&normals, INITIAL_NORMALS_CAPACITY);
    initTriArray(&faces, INITIAL_FACES_CAPACITY);

    char line[1024]; 
    size_t line_num = 0;

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        if (line[0] == '\n' || line[0] == '\r') continue;

        if (line[0] == 'v') {
            if (line[1] == ' ') { 
                vec3 vertex;
                if (sscanf(line + 2, "%f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
                    appendVec3(&vertices, vertex);
                } else {
                    fprintf(stderr, "Failed to parse vertex at line %zu: %s", line_num, line);
                }
            }
            else if (line[1] == 't') { 
                vec2 texcoord;
                if (sscanf(line + 3, "%f %f", &texcoord.x, &texcoord.y) == 2) {
                    appendVec2(&texcoords, texcoord);
                } else {
                    fprintf(stderr, "Failed to parse texcoord at line %zu: %s", line_num, line);
                }
            }
            else if (line[1] == 'n') {
                vec3 normal;
                if (sscanf(line + 3, "%f %f %f", &normal.x, &normal.y, &normal.z) == 3) {
                    appendVec3(&normals, normal);
                } else {
                    fprintf(stderr, "Failed to parse normal at line %zu: %s", line_num, line);
                }
            }
        }
        else if (line[0] == 'f') {
            char *token = strtok(line + 2, " \t\r\n");
            int vertex_indices[4] = {0};
            int texcoord_indices[4] = {0};
            int normal_indices[4] = {0};
            int vertex_count = 0;

            while (token != NULL && vertex_count < 4) { 
                if (!parseFaceVertex(token, &vertex_indices[vertex_count], &texcoord_indices[vertex_count], &normal_indices[vertex_count])) {
                    fprintf(stderr, "Unexpected face vertex format at line %zu: %s\n", line_num, token);
                    break; 
                }
                vertex_count++;
                token = strtok(NULL, " \t\r\n");
            }

            if (vertex_count < 3) {
                fprintf(stderr, "Incomplete face at line %zu\n", line_num);
                continue; 
            }

            for (int i = 1; i < vertex_count - 1; i++) {
                tri triangle;
                triangle.vx = vertices.data[vertex_indices[0] - 1];
                triangle.vy = vertices.data[vertex_indices[i] - 1];
                triangle.vz = vertices.data[i + 1 < vertex_count ? vertex_indices[i + 1] - 1 : vertex_indices[0] - 1];

                if (texcoords.size > 0) {
                    if (texcoord_indices[0] > 0 && texcoord_indices[i] > 0 && texcoord_indices[i + 1] > 0) {
                        triangle.uvx = texcoords.data[texcoord_indices[0] - 1];
                        triangle.uvy = texcoords.data[texcoord_indices[i] - 1];
                        triangle.uvz = texcoords.data[i + 1 < vertex_count ? texcoord_indices[i + 1] - 1 : texcoord_indices[0] - 1];
                    } else {
                        triangle.uvx = VEC2(0, 0);
                        triangle.uvy = VEC2(0, 0);
                        triangle.uvz = VEC2(0, 0);
                    }
                } else {
                    triangle.uvx = VEC2(0, 0);
                    triangle.uvy = VEC2(0, 0);
                    triangle.uvz = VEC2(0, 0);
                }

                if (normals.size > 0) {
                    if (normal_indices[0] > 0 && normal_indices[i] > 0 && normal_indices[i + 1] > 0) {
                        triangle.nvx = normals.data[normal_indices[0] - 1];
                        triangle.nvy = normals.data[normal_indices[i] - 1];
                        triangle.nvz = normals.data[i + 1 < vertex_count ? normal_indices[i + 1] - 1 : normal_indices[0] - 1];
                    } else {
                        triangle.nvx = VEC3(0, 0, 0);
                        triangle.nvy = VEC3(0, 0, 0);
                        triangle.nvz = VEC3(0, 0, 0);
                    }
                } else {
                    triangle.nvx = VEC3(0, 0, 0);
                    triangle.nvy = VEC3(0, 0, 0);
                    triangle.nvz = VEC3(0, 0, 0);
                }

                appendTri(&faces, triangle);
            }
        }
    }

    fclose(f);

    texture loaded_texture = loadimg(imgpath);

    mesh new_mesh;
    new_mesh.tris = (tri *)malloc(faces.size * sizeof(tri));
    if (!new_mesh.tris) {
        perror("Failed to allocate memory for mesh faces.");
        exit(EXIT_FAILURE);
    }
    memcpy(new_mesh.tris, faces.data, faces.size * sizeof(tri));
    new_mesh.tri_count = (int)faces.size;
    new_mesh.tex = (texture *)malloc(sizeof(texture));
    if (!new_mesh.tex) {
        perror("Failed to allocate memory for mesh texture.");
        exit(EXIT_FAILURE);
    }
    *(new_mesh.tex) = loaded_texture;

    free(vertices.data);
    free(texcoords.data);
    free(normals.data);
    free(faces.data);

    printf("Successfully loaded OBJ file: %s\n", path);
    printf("Vertices: %zu, Texcoords: %zu, Normals: %zu, Faces: %zu\n",
           vertices.size, texcoords.size, normals.size, faces.size);

    return new_mesh;
}

/**
 * Absolutely transform a mesh
 */
void meshabs(mesh A, float B[4][4]){
    for (int i = 0; i < A.tri_count; i++){
        tri t = A.tris[i];
        A.tris[i].vx = matvec(t.vx, B);
        A.tris[i].vy = matvec(t.vy, B);
        A.tris[i].vz = matvec(t.vz, B);
    }
}

/** 
 * General mouse/keyboard function handler 
 */
void ehandle(display *d, POINT p0) {

    float speed = d->camera.speed;
    float rotationSpeed = 0.05f;

    if (GetAsyncKeyState('W') & KEY_PRESSED) {
        vec3 forward = { -d->camera.matrot[2][0], -d->camera.matrot[2][1], -d->camera.matrot[2][2] };
        vec3normalize(&forward);
        d->camera.pos.x += forward.x * speed;
        d->camera.pos.y += forward.y * speed;
        d->camera.pos.z += forward.z * speed;
    }
    if (GetAsyncKeyState('S') & KEY_PRESSED) {
        vec3 backward = { d->camera.matrot[2][0], d->camera.matrot[2][1], d->camera.matrot[2][2] };
        vec3normalize(&backward);
        d->camera.pos.x += backward.x * speed;
        d->camera.pos.y += backward.y * speed;
        d->camera.pos.z += backward.z * speed;
    }
    if (GetAsyncKeyState('A') & KEY_PRESSED) {
        vec3 left = { -d->camera.matrot[0][0], -d->camera.matrot[0][1], -d->camera.matrot[0][2] };
        vec3normalize(&left);
        d->camera.pos.x += left.x * speed;
        d->camera.pos.y += left.y * speed;
        d->camera.pos.z += left.z * speed;
    }
    if (GetAsyncKeyState('D') & KEY_PRESSED) {
        vec3 right = { d->camera.matrot[0][0], d->camera.matrot[0][1], d->camera.matrot[0][2] };
        vec3normalize(&right);
        d->camera.pos.x += right.x * speed;
        d->camera.pos.y += right.y * speed;
        d->camera.pos.z += right.z * speed;
    }
    if (GetAsyncKeyState(VK_SPACE) & KEY_PRESSED) {
        d->camera.pos.y += speed;
    }
    if (GetAsyncKeyState(VK_SHIFT) & KEY_PRESSED) {
        d->camera.pos.y -= speed;
    }
    if (GetAsyncKeyState(VK_LEFT) & KEY_PRESSED) {
        d->camera.yaw += rotationSpeed;
    }
    if (GetAsyncKeyState(VK_RIGHT) & KEY_PRESSED) {
        d->camera.yaw -= rotationSpeed;
    }
    if (GetAsyncKeyState(VK_UP) & KEY_PRESSED) {
        d->camera.pitch += rotationSpeed;
    }
    if (GetAsyncKeyState(VK_DOWN) & KEY_PRESSED) {
        d->camera.pitch -= rotationSpeed;
    }
    if (GetAsyncKeyState('I') & KEY_PRESSED) {
        d->camera.speed += .1;
    }
    if (GetAsyncKeyState('O') & KEY_PRESSED) {
        d->camera.speed -= (d->camera.speed > 0.1) ? .1 : 0;
    }

    POINT p1;

    if (GetCursorPos(&p1)){
        d->camera.yaw += (-p1.x + p0.x) * MOUSE_DELTA_SENSITIVITY;
        d->camera.pitch += (-p1.y + p0.y) * MOUSE_DELTA_SENSITIVITY;
    }

    float pitch[4][4], yaw[4][4];
    matrtx(d->camera.pitch, pitch);
    matrty(d->camera.yaw, yaw);

    matmul(pitch, yaw, d->camera.matrot);
}

/**
 * Camera constructor.
 */
cam ccam(vec3 pos, float theta, float fov, float aspect, float fnear, float ffar, float speed){
    cam new_cam;
    new_cam.pos = pos;
    new_cam.theta = theta;
    new_cam.fov = fov;
    new_cam.aspect = aspect;
    new_cam.fnear = fnear;
    new_cam.ffar = ffar;
    new_cam.matrot[0][0] = 1.0f;
    new_cam.matrot[1][1] = 1.0f;
    new_cam.matrot[2][2] = 1.0f;
    new_cam.matrot[3][3] = 1.0f;
    new_cam.yaw = 0.0f;
    new_cam.pitch = 0.0f;
    new_cam.speed = speed;
    return new_cam;
}

/**
 * Display constructor.
 */
display cdisp(cam camera, int display_width, int display_height, vec3 color){
    display new_display;
    new_display.meshes = NULL;
    new_display.mesh_count = 0;
    new_display.camera = camera;
    new_display.display_width = display_width;
    new_display.display_height = display_height;
    new_display.background = color;
    return new_display;
}

/**
 * Adds a mesh to the display.
 */
void mshadd(display *d, mesh new_mesh) {
    d->meshes = (mesh *)realloc(d->meshes, (d->mesh_count + 1) * sizeof(mesh));
    
    if (d->meshes == NULL) {
        fprintf(stderr, "Memory allocation failed in mshadd.\n");
        exit(EXIT_FAILURE);
    }

    d->meshes[d->mesh_count] = new_mesh;

    d->mesh_count++;
}

/** 
 * Creates a system call to fetch the console window's size 
*/
window winsize(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    window new_window = {SCREEN_WIDTH, SCREEN_HEIGHT};
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
void wininit(window wprop){
    ShowCursor(FALSE);
    SetConsoleOutputCP(CP_UTF8);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT window_size = {0, 0, wprop.width, wprop.height};
    SetConsoleWindowInfo(hConsole, TRUE, &window_size);
    COORD buffer_size = {wprop.width, wprop.height};
    SetConsoleScreenBufferSize(hConsole, buffer_size);
    SYS_ANSI_RESET;
}

/*
* -
* /Note software implementations do not need to use main()\
*	as their base
* -
* Declaration of the standard C library main() function\
*   defines the overall custom user configs for the package
*/
