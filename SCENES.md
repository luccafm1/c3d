## Scene File Structure

A **`.scene`** file (e.g., `myscene.scene`) is an INI-style definition that configures how the engine loads in a more user-friendly manner.

In short, there are 5 sections to cover regarding scenes:

- [camera];
- [meshes];
- [display];
- [lights];
- [startup]/[continuous].

### Camera Section

```ini
[camera]
position 0 2 5   # camera starts at x=0, y=2, z=5
fov 70           # field of view
speed 0.5        # movement speed for WASD navigation
```

### Meshes Section

```ini
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
```ini
assets/models/<folder_name>/any.obj   (required)
assets/models/<folder_name>/any.png   (optional*)
assets/models/<folder_name>/any.mtl   (optional)
```
Or whatever your `C3D_REL_MODELS_PATH` macro is defined to.

> (*) though models can **technically** run without *.png* files, the resulting render may not be optimal.

### Display Section

```ini
[display]
background_color 30 30 30

# Sets the R, G, B of the background color (0-255).
```

### Lights Section

```ini
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

**Startup** behaviors here run **once** at scene initialization (first frame only).

**Continuous** behaviors here run **every frame**. 

Within `[startup]` or `[continuous]`, each line is parsed as a command,

```ini
[startup]
rotate spaceship Y 90                # Rotate `spaceship` by 90 degrees around Y **once** at the start
movetomesh character spaceship 0.5   # Move `character` mesh a little toward `spaceship` center

[continuous]
rotate planet Y 1                    # Rotate `planet` mesh by 1 degree around Y **each frame** (creates spinning).
swaptex billboard billboard_alt.png  # Swap texture of `billboard` every frame (this is just an example—really you might want to do it only once or conditionally).
```

---

`rotate <meshName|ALL> <X|Y|Z> <angle>`  Rotates a given mesh (or **ALL** meshes) around the specified axis by `<angle>` degrees each frame (if `[continuous]`).

`movetomesh <sourceMesh> <targetMesh> <step>`  Moves `<sourceMesh>` closer to `<targetMesh>` by `<step>` each frame (direction is from source center to target center).

`moveto <meshName> <targetX> <targetY> <targetZ> <step>`  Moves `<meshName>` incrementally towards a target position `<targetX,targetY,targetZ>`.

`swaptex <meshName> <path/to/new/texture.png>`  Changes the diffuse texture for `<meshName>` to the provided path. 

`swapmesh <oldMeshName> <folderName>`  Replaces the given mesh with a new one loaded from `assets/models/<folderName>`.

`rotate_id <meshID> <X|Y|Z> <angle>`  Same as `rotate` but uses integer mesh ID (index order loaded) instead of mesh name.

`movetomesh_id <sourceID> <targetID> <step>`   Same as `movetomesh` but references mesh IDs.

`moveto_id <meshID> <x> <y> <z> <step>`  Same as `moveto`, but references mesh ID.

`swaptex_id <meshID> <path/to/new/texture>` Same as `swaptex` but uses mesh ID.

`swapmesh_id <meshID> <folderName>`  Same as `swapmesh` but uses mesh ID.

`scalemesh <meshID> <sx> <sy> <sz>`  Scales a mesh by the factors provided.

`colorize <meshID> <R> <G> <B>`  A quick method to set the diffuse texture of a mesh to a solid color (experimental).

### Final considerations and results

At the very end, your scene file should look something like this:

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
