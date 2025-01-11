// main.c

// this is an example application file.

#define C3D_STANDARD
#define FORCE_SMOOTH    // forces all vectors to adopt vector-normal averaging, which makes lighting more stable
#include "c3d.h"

#include <stdio.h>
#include <stdlib.h>

int main(void){
    window size = c3d_winsize();
    c3d_wininit(size);

    cam c = c3d_initcam((vec3){0.0f, 0.0f, 0.0f},   // position
                    70.0f,                      // fov
                    0.1f                        // speed
                    );

    display d = c3d_initdisplay(c,                          // the first-person camera
                            size.width - 5,             // initial width (-5 to prevent overflow) 
                            size.height - 5,            // initial height (-5 to prevent overflow)
                            (vec3){0.0f, 0.0f, 0.0f}    // initial background color
                            );  

    while (d.running){
        if (GetAsyncKeyState(VK_ESCAPE) & C3D_KEY_PRESSED) {
            c3d_retgui(&d); // opens the standard rectangular GUI if ESC key is pressed
        }

        POINT p0;
        GetCursorPos(&p0);

        float fps = c3d_getavgfps();
        fprintf(stdout, "FPS: %.2f", fps);

        c3d_auto_winres(&d, &c); // updates display resolution

        c3d_update(&d); // updates screen

        c3d_k_handle(&d);       // handle keyboard events 
        c3d_m_handle(&d, p0);   // handle mouse events 
    }

    return 0;
}
