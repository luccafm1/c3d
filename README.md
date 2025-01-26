
# C3D

**C3D** is a C99+ 3D software-renderer pipeline built for the Windows API.

As of the latest version, texture & image loading is done using the [`stb_image.h`](https://github.com/nothings/stb) API. 

To use C3D as a standalone API without texture loading:

```C
#define C3D__NO_STBI
#include "<your_path>/c3d.h"
```

---

## Installation

From the terminal,

1. **clone the repo,**

   ```bash
   git clone https://github.com/luccafm1/c3d.git
   cd c3d
   ```

2. **build,** (using the included Makefile or your own build system):

   ```bash
   make
   ```
   ...or compile manually:
   ```bash
   gcc -Wall -o c3d ./src/main.c -lm
   ```

3. **run** (`make run` also works for steps 2 and 3):

   ```bash
   bin/c3d.exe
   ```

---

## Standard C3D (Standard implementation)

**C3D (`c3d.h`) works standalone with STB_IMAGE** as an API well as within its own integrated system.  

The integrated system is included in the header file and is accessible through defining the standard implementation macro:

```C
#define C3D_STANDARD
#include "<your_path>/c3d.h" 
```

You will see a menu that allows for many helpers, but these preclude a specific project structure. You load a scene either from `assets/scenes/*` or a `.obj` folder from `assets/models/*`.

These locations are defined from the relative paths in the macros:

```C
#define C3D_REL_SCENES_READ_PATH   /* ... */
#define C3D_REL_MODELS_READ_PATH   /* ... */
#define C3D_MODELS_READ_PATH       /* ... */
```

They expect a project structure similar to this:

```
c3d/
├─ include/
│  ├─ c3d.h
│  └─ stb_image.h
├─ src/
│  └─ main.c
└─ assets/
   ├─ models/
   | ├─ some_model/
   | │  ├─ main.obj
   | │  └─ diffuse.png
   └─ scenes/
      └─ myscene
```

---

## License

This project is released under the [MIT License](LICENSE). In short:

```
Permission is hereby granted, free of charge, to any person obtaining a copy of this software ...
```

You may freely use, modify, distribute, etc., for commercial or non-commercial purposes.

---

## Disclaimer

C3D is a minimal demonstration of 3D rendering in a Windows terminal environment and isn’t optimized for production or performance. No guarantee is provided regarding compatibility, correctness, or stability. Use at your own risk and have fun!
