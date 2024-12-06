// main.c

// this is an example application file.

#include "c3d.h"
#include "load.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int main(int argc, char**argv){
    if (argc > 7){
        fprintf(stderr, "Incorrect amount of arguments. ");
        exit(EXIT_FAILURE);
    }

    window size = winsize();
    wininit(size);

    display d = cdisp(CAM(VEC3(0, 0, 0), 70, .5), size.width - 5, size.height - 5, VEC3(0, 0, 0));

    _retgui(&d);

    while (1){
        POINT p0;
        GetCursorPos(&p0);

        if (GetAsyncKeyState(VK_ESCAPE) & KEY_PRESSED) {
            _retgui(&d);
        }

        window size = winsize();
        d.display_width = size.width - 5;
        d.display_height = size.height - 5;

        update(&d);
        ehandle(&d, p0);
        
        Sleep(1);
    }

    for (int i = 0; i < d.mesh_count; i++) {
        free(d.meshes[i].tris);
    }
    free(d.meshes);

    return 0;
}
