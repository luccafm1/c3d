
# C3D – A Lightweight 3D Renderer for the Windows Terminal

**C3D** is a proof-of-concept 3D rendering engine implemented entirely in C for the Windows console. It leverages the Windows API for console manipulation, uses [`stb_image.h`](https://github.com/nothings/stb) for texture loading, and renders 3D objects directly in the terminal using a simple rasterizer.

> **Note**: This is an experimental software renderer. Bigger projects typically require hardware solutions like the ones provided by OpenGL. 

## Table of Contents

1. [Features](#features)  
2. [Quick Start](#quick-start)  
3. [Project Layout](#project-layout)  
4. [Building](#building)  
5. [Running](#running)  
6. [Scene File Structure](#scene-file-structure)  
   - [Camera Section](#camera-section)  
   - [Meshes Section](#meshes-section)  
   - [Display Section](#display-section)  
   - [Lights Section](#lights-section)  
   - [Behaviors: Startup & Continuous](#behaviors-startup--continuous)  
     - [Available Behavior Commands](#available-behavior-commands)  
7. [Controls](#controls)  
8. [License](#license)  
9. [Disclaimer](#disclaimer)

---

## Features

- **Windows-only**: Relies on the Windows Console API for rasterization.
- **Simple .OBJ Loading**: Quickly load models with optional `.mtl` materials.
- **Texture Support**: Uses `stb_image.h` for loading `.png`, `.jpg`, etc.
- **Scene Files**: Define camera, background color, lights, and behaviors in `.scene` files.
- **Built-in Menu**: Interactively load either a scene file (under `assets/scenes/`) or a single `.obj` folder (under `assets/models/`).

---

## Quick Start

1. **Clone the Repo:**

   ```bash
   git clone https://github.com/luccafm1/c3d.git
   cd c3d
   ```

2. **Build** (using the included Makefile or your own build system):

   ```bash
   make
   ```
   Or compile manually:
   ```bash
   gcc -Wall -o c3d ./src/main.c -lm
   ```

3. **Run**:

   ```bash
   bin/c3d.exe
   ```

Upon launching, you will see a menu that helps you load a scene from `assets/scenes/*` or a `.obj` folder from `assets/models/*`.

---

## Project Layout

```
c3d/
├─ include/
│  ├─ c3d.h
│  └─ stb_image.h
├─ src/
│  └─ main.c
├─ assets/
|  ├─ models/
│  | ├─ some_model/
│  | │  ├─ main.obj
│  | │  └─ diffuse.png
|  └─ scenes/
|     └─ myscene
├─ bin/
│  └─ c3d.exe
├─ Makefile
├─ README.md (this file)
└─ LICENSE
```

---

## Building

A basic **Makefile** is provided. Run `make` in the root directory to build the `c3d` executable:

```bash
make
# or
gcc -Wall -o c3d ./src/main.c -lm
```

Ensure `c3d.h` and `stb_image.h` are in your compiler’s include path if you’re customizing your build commands.

---

## Running

After a successful build, an executable `c3d.exe` (Windows) will appear under `bin/`. Run it from the project root:

```bash
bin/c3d.exe
```

You’ll be presented with a menu:

- **Left/Right Arrow**: Toggle between selecting Scenes or Models.  
- **Up/Down Arrow**: Navigate the list of scene files or model folders.  
- **Enter**: Load the highlighted scene or model folder.  
- **Escape**: Exit the menu.

Once a scene or model folder is loaded, the engine will render it in the console.

---

## Scene File Structure

A **`.scene`** file (e.g., `myscene`) is an INI-style definition that configures how the engine loads:

- [camera]: Set camera position, field of view, etc.
- [meshes]: Define which models to load and how to position and scale them.
- [display]: Customize background color or other properties.
- [lights]: Create one or more lights in the scene.
- [startup]/[continuous]: Define behaviors that run once (startup) or every frame (continuous).

Below is a sample `.scene` file:

```ini
[camera]
position 0 2 5
fov 70
speed 0.5

[meshes]
my_model 0 0 0 1.0 1.0 1.0

[display]
background_color 30 30 30

[lights]
0 2 2 255 255 255 1.0 2.0

[startup]
rotate my_model Y 45

[continuous]
rotate my_model Y 1
```

### Camera Section

```
[camera]
position 0 2 5   # camera starts at x=0, y=2, z=5
fov 70           # field of view
speed 0.5        # movement speed for WASD navigation
```

### Meshes Section

```
[meshes]
my_model 0 0 0 1.0 1.0 1.0

# The syntax is:
# <folder_name> <posX> <posY> <posZ> <scaleX> <scaleY> <scaleZ>
#
# - folder_name: subfolder under ./assets/models/
# - posX, posY, posZ: where to place the mesh in the scene
# - scaleX, scaleY, scaleZ: how to scale the mesh
```

When you specify a “folder_name” under `[meshes]`, C3D looks for:
```
assets/models/<folder_name>/any.obj   (required)
assets/models/<folder_name>/any.png   (optional*)
assets/models/<folder_name>/any.mtl   (optional)
```
> though models can **technically** run without *.png* files, the resulting render may not be optimal.

### Display Section

```
[display]
background_color 30 30 30

# Sets the R, G, B of the background color (0-255).
```

### Lights Section

```
[lights]
0 2 2 255 255 255 1.0 2.0
-1 3 4 255 0 0 1.0 1.5

# Syntax per line:
# X Y Z R G B brightness radius
#
#  - (X, Y, Z): position of the light
#  - (R, G, B): color of the light (each 0–255)
#  - brightness: multiplier for the light intensity
#  - radius: approximate range for attenuation
```

Each line in `[lights]` defines one point light source in the scene. You can define multiple lights by adding additional lines.

---

### Behaviors: Startup & Continuous

C3D supports *behaviors* in your scene definition. Behaviors are simple commands that automatically manipulate meshes (rotate, move, swap textures, etc.) when the scene runs:

- **[startup]**: Behaviors here run **once** at scene initialization (i.e., on the first frame only).
- **[continuous]**: Behaviors here run **every frame**. This is useful for animations, real-time transformations, etc.

#### Available Behavior Commands

Within `[startup]` or `[continuous]`, each line is parsed as:

1. `rotate <meshName|ALL> <X|Y|Z> <angle>`  
   - Rotates a given mesh (or **ALL** meshes) around the specified axis by `<angle>` degrees each frame (if `[continuous]`).
2. `movetomesh <sourceMesh> <targetMesh> <step>`  
   - Moves `<sourceMesh>` closer to `<targetMesh>` by `<step>` each frame (direction is from source center to target center).
3. `moveto <meshName> <targetX> <targetY> <targetZ> <step>`  
   - Moves `<meshName>` incrementally towards a target position `<targetX,targetY,targetZ>`.
4. `swaptex <meshName> <path/to/new/texture.png>`  
   - Changes the diffuse texture for `<meshName>` to the provided path. 
5. `swapmesh <oldMeshName> <folderName>`  
   - Replaces the given mesh with a new one loaded from `assets/models/<folderName>`.
6. `rotate_id <meshID> <X|Y|Z> <angle>`  
   - Same as `rotate` but uses integer mesh ID (index order loaded) instead of mesh name.
7. `movetomesh_id <sourceID> <targetID> <step>`  
   - Same as `movetomesh` but references mesh IDs.
8. `moveto_id <meshID> <x> <y> <z> <step>`  
   - Same as `moveto`, but references mesh ID.
9. `swaptex_id <meshID> <path/to/new/texture>`  
   - Same as `swaptex` but uses mesh ID.
10. `swapmesh_id <meshID> <folderName>`  
    - Same as `swapmesh` but uses mesh ID.
11. `loopmesh <meshID> <frameCount>`  
    - An experimental command that attempts to load mesh frames in a loop. (Requires a particular naming scheme, e.g., `myMesh0.obj`, `myMesh1.obj`, etc.)
12. `scalemesh <meshID> <sx> <sy> <sz>`  
    - Scales a mesh by the factors provided.
13. `colorize <meshID> <R> <G> <B>`  
    - A quick method to set the diffuse texture of a mesh to a solid color (experimental).

#### Example Behavior Usage

```
[startup]
rotate spaceship Y 90
movetomesh character spaceship 0.5

[continuous]
rotate planet Y 1
swaptex billboard billboard_alt.png
```

- **startup**: 
  - Rotate `spaceship` by 90 degrees around Y **once** at the start.
  - Move `character` mesh a little toward `spaceship` center.

- **continuous**: 
  - Rotate `planet` mesh by 1 degree around Y **each frame** (creates spinning).
  - Swap texture of `billboard` every frame (this is just an example—really you might want to do it only once or conditionally).

---

## Controls

When the scene is running (in the rendered console view):
- **W, A, S, D**: Move camera forward, left, backward, right
- **Arrow Keys**: Rotate camera (yaw/pitch)
- **Space**: Move camera up
- **Shift**: Move camera down
- **I / O**: Increase or decrease camera speed
- **Mouse**: Move mouse left/right/up/down to rotate camera
- **Enter or Left Mouse Click**: Creates a new random light source at the camera position (demo feature)
- **Escape**: Return to the main menu

---

## License

This project is released under the [MIT License](LICENSE). In short:

```
Permission is hereby granted, free of charge, to any person obtaining a copy of this software ...
```

You may use, modify, distribute, etc., for commercial or non-commercial purposes.

---

## Disclaimer

C3D is a minimal demonstration of 3D rendering in a Windows terminal environment and isn’t optimized for production or performance. No guarantee is provided regarding compatibility, correctness, or stability. Use at your own risk and have fun!
