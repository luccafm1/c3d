# C3D – A Lightweight 3D renderer for the Windows Terminal

C3D is a proof-of-concept 3D rendering engine implemented entirely in C for the Windows console. It leverages the Windows API for console manipulation, uses `stb_image.h` for texture loading, and renders 3D objects directly into the terminal with some rudimentary rasterization techniques. 

**Note:** This is an experimental engine. Performance, features, and stability are limited compared to mature 3D engines.

## Models/scenes

C3D provides a built-in interactive menu in load.h, which can be imported into your project. It searches for an `assets` directory containing two folders: `models` and `scenes`. The structure and layout of each model and scene is as follows:

- **Scenes:**  
  Stored in the `scenes/` directory. A `.scene` file can define camera position, FOV, background color, and references to objects along with their transforms (translation, scaling).
  
- **Object Folders:**  
  Under `models/`, each folder must contain a `main.obj` file and a `diffuse.png` texture. The engine attempts to load these at runtime.

**Example:**
```
assets/scenes/
└─ myscene.scene

assets/models/
└─ mymodel/
   ├─ main.obj
   └─ diffuse.png
```

In `myscene.scene`, you should follow the syntax:
```ini
[camera]
position 0 2 5
fov 70
speed 0.5

[meshes]
mymodel 0 0 0 1.0

[display]
background_color 30 30 30
```

## Directory Layout

```
c3d/
├─ include/
│  ├─ c3d.h
│  └─ stb_image.h
├─ src/
│  └─ main.c
├─ assets/
│  ├─ some_model/   (example model file)
│  │  │  ├─ main.obj
│  │  │  └─ diffuse.png
├─ README.md (this file)
└─ LICENSE (MIT or Unlicense)
```

## Dependencies & Requirements

- **Windows OS:**  
  C3D relies on the Windows Console API, so it runs on Windows.
  
- **Compiler:**  
  A C compiler that supports C99 or later. Common choices:
  - Microsoft Visual Studio (MSVC)
  - MinGW/GCC
  
- **stb_image:**  
  The project includes `stb_image.h` (header-only library) for image loading.
  
- **Console Capabilities:**  
  The project uses ANSI escape codes for color and positional control. Ensure your Windows terminal supports these escape sequences (Windows 10+ or Windows Terminal recommended).

## Building the Project

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/luccafm1/c3d.git
   cd c3d
   ```

2. **Compile:**
   This project uses `Makefile` as default. Run it in the project directory as:
   ```bash
   make
   ```
   
   Otherwise, compile manually:
   ```bash
   gcc -Wall -o c3d ./src/main.c -lm
   ```
   
   Make sure `c3d.h` and `stb_image.h` are in your include path (`-Iinclude` if needed).

3. **Run:**
   Make sure to run the following command from the project directory's root folder:
   ```bash
   bin/c3d.exe
   ```

## Base camera controls

- **W, A, S, D**: Move forward, left, backward, right.
- **ARROW KEYS**: Rotate camera (left/right/yaw, up/down/pitch).
- **SPACE**: Move camera up.
- **SHIFT**: Move camera down.
- **I, O**: Increase/decrease camera speed.
- **MOUSE**: Move to adjust yaw/pitch.
- **ESC**: Return to the menu.

## Licensing

The code is provided under both the MIT License and a Public Domain dedication as stated in the headers:

**MIT License:**
```
Permission is hereby granted, free of charge, to any person obtaining a copy of this software ...
```

**Public Domain (Unlicense):**
```
This is free and unencumbered software released into the public domain.
...
```

You are free to use, modify, and distribute C3D for any purpose.

## Disclaimer

C3D is a minimal and experimental implementation of 3D rendering in a Windows terminal environment. It’s not optimized for real performance or production use. There are no guarantees of compatibility or stable APIs.

**Have fun!**