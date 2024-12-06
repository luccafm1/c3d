//load.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define REL_SCENES_READ_PATH "./assets/scenes/*"
#define REL_OBJ_READ_PATH "./assets/models/*"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

/**
 * Concatenates 3 strings together.
 */
char *_strcat_3(const char *prefix, const char *string, const char *suffix){
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

/**
 * Lists file in a directory
 */
void filelist(const char *path, char ***out, int *count) {
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
 * Lists folders in a directory
 */
void folderlist(const char *path, char ***out, int *count) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No directories found in the %s path.\n", path);
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
 * Resets a display struct
 */
void dispreset(display *d) {
    d->background.x = 0;
    d->background.y = 0;
    d->background.z = 0;

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
    d->mesh_count = 0; 
}

/**
 * Loads folder from display
 */
int loadobjfolder(display *d, const char *folder) {
    dispreset(d);

    const char *pfix = "assets/models/";

    char *obj_path = _strcat_3(pfix, folder, _OBJ_FILE);
    char *img_path = _strcat_3(pfix, folder, _OBJ_IMG_FILE);

    mesh new_mesh = loadobj(obj_path, img_path);
    mshadd(d, new_mesh);

    free(obj_path);
    free(img_path);
    return 1;
}

/**
 * Loads scene from path into a display.
 */
int loadscene(display *d, char *path){
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    }
    
    dispreset(d);

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
            float x, y, z, scale;

            if (sscanf_s(line, "%49s %f %f %f %f", mpath, (unsigned)_countof(mpath), &x, &y, &z, &scale) == 5){

                const char *pfix = "assets/models/";

                char* obj_path = _strcat_3(pfix, mpath, _OBJ_FILE);
                char* img_path = _strcat_3(pfix, mpath, _OBJ_IMG_FILE);

                mesh new_mesh = loadobj(obj_path, img_path);

                float translate[4][4];
                mattra(x, y, z, translate);

                float mscale[4][4];
                matscl(scale, scale, scale, mscale);

                float transform[4][4];
                matmul(translate, mscale, transform);
                
                mshabs(&new_mesh, transform);

                mshadd(d, new_mesh);

                free(obj_path);
                free(img_path);
            }
            
        }
        if (!strcmp(buffer, "display")){

            char key[50];
            int r, g, b;

            if (sscanf_s(line, "%49s %d %d %d", key, (unsigned)_countof(key), &r, &g, &b) == 4){
                if (!strcmp(key, "background_color")){ 
                    d->background.x = r; 
                    d->background.y = g; 
                    d->background.z = b; 
                }
            }
        }
        if (!strcmp(buffer, "behavior")){
            /* . . . */

            /**
             * Behaviors are function calls which are called every display update.
             * 
             * rotate ALL X 1            # rotates all meshes around the x axis by theta=1
             * rotate Cube Y 5           # rotates a mesh Cube around the y axis by theta=5
             * moveto Charizard Trex .1  # moves a mesh Charizard towards Trex by 0.1
             */
            
        }
    }

    fclose(f);

    return 1;
}

void show_file_contents(const char *filename) {
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

int hastexture(const char *folder) {
    char path1[256], path2[256];

    snprintf(path1, sizeof(path1), "assets/models/%s/diffuse.png", folder);
    snprintf(path2, sizeof(path2), "assets/models/%s/diffuse.jpg", folder);
    
    if (_access(path1, 0) == 0 || _access(path2, 0) == 0) {
        return 1; 
    }

    return 0;
}

/**
 * Built-in menu for ease of interaction
 * 
 * Reads user input and returns the option. 
 */
void _retgui(display *d){
    char **paths;
    char **folderpaths;
    int count;
    int foldercount;

    filelist(REL_SCENES_READ_PATH, &paths, &count);
    folderlist(REL_OBJ_READ_PATH, &folderpaths, &foldercount);

    char*menu =
    "\033[H"
    "╔═════════════════════════════════╗\n"
    "║                                 ║\n"
    "║ ░█████╗░░░░░░░░██████╗░██████╗░ ║\n"
    "║ ██╔══██╗░░██╗░░╚════██╗██╔══██╗ ║\n"
    "║ ██║░░╚═╝██████╗░█████╔╝██║░░██║ ║\n"
    "║ ██║░░██╗╚═██╔═╝░╚═══██╗██║░░██║ ║\n"
    "║ ╚█████╔╝░░╚═╝░░██████╔╝██████╔╝ ║\n"
    "║ ░╚════╝░░░░░░░░╚═════╝░╚═════╝░ ║\n"
    "║                                 ║\n"
    "║    Use LEFT/RIGHT to switch,    ║\n"
    "║  UP/DOWN to navigate, ENTER to  ║\n"
    "║     select, ESCAPE to EXIT!     ║\n"
    "║                                 ║\n";

    int current = 0;
    int type = 0;

    Sleep(100);
    while(1){
        fprintf(stdout, menu);
        fprintf(stdout, (type == 0) ? "║      Reading scenes files       ║\n" : "║       Reading .OBJ files        ║\n");
        fprintf(stdout, "║                                 ║\n");

        for (int i = 0; i < ((type == 0) ? count : foldercount); i++) {
            const char *foldername = (type == 0) ? paths[i] : folderpaths[i];
            int has_texture = (type == 1) ? hastexture(foldername) : 1;
            if (i == current) {
                if (!has_texture) {
                    fprintf(stdout, "║%s <X>\t%-20s\t%s  ║\n", ACOLOR(255, 100, 100), foldername, RESET_COLOR);
                } else {
                    fprintf(stdout, "║%s <X>\t%-20s\t%s  ║\n", ACOLOR(255, 255, 255), foldername, RESET_COLOR);
                }
            } else {
                if (!has_texture) {
                    fprintf(stdout, "║%s < >\t%-20s\t%s  ║\n", ACOLOR(255, 100, 100), foldername, RESET_COLOR);
                } else {
                    fprintf(stdout, "║ < >\t%-20s\t  ║\n", foldername);
                }
            }
        }

        fprintf(stdout, "╚═════════════════════════════════╝");
        if (GetAsyncKeyState(VK_LEFT) & KEY_PRESSED) {
            type = (type == 0) ? 1 : 0;
            current = 0;
            Sleep(150);
            SYS_ANSI_RESET;
        }
        if (GetAsyncKeyState(VK_RIGHT) & KEY_PRESSED) {
            type = (type == 1) ? 0 : 1;
            current = 0;
            Sleep(150);
            SYS_ANSI_RESET;
        }
        if (GetAsyncKeyState(VK_UP) & KEY_PRESSED) {
            current = (current == 0) ? ((type == 0) ? count: foldercount) - 1 : current - 1;
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_DOWN) & KEY_PRESSED) {
            current = (current == ((type == 0) ? count: foldercount)  - 1) ? 0 : current + 1;
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_RETURN) & KEY_PRESSED) {
            Sleep(100);

            if (type == 0){
                char full_path[256];
                snprintf(full_path, sizeof(full_path), "./assets/scenes/%s", paths[current]);
                SYS_ANSI_RESET;
                fprintf(stdout, "Contents of %s:\n\n", paths[current]);
                show_file_contents(full_path);

                fprintf(stdout, "\n\n|RETURN| Continue\t\t|ESCAPE| Back\n");

                while(1){
                    if (GetAsyncKeyState(VK_RETURN) & KEY_PRESSED){
                        SYS_ANSI_RESET;
                        loadscene(d, full_path);
                        return;
                    }
                    if (GetAsyncKeyState(VK_ESCAPE) & KEY_PRESSED){
                        SYS_ANSI_RESET;
                        Sleep(100);
                        break;
                    }
                }
            } else {
                SYS_ANSI_RESET;
                fprintf(stdout, "Loading Object from %s...\n", folderpaths[current]);
                loadobjfolder(d, folderpaths[current]);
                return;
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & KEY_PRESSED){
            Sleep(100);
            break;
        }
    }
}