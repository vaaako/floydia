# Floydia 🪲 v1.0
**Floydia** is a C++ OpenGL 4.5 graphics library designed to simplify 2D and 3D development. It provides an easy-to-use, beginner-friendly API while still offering powerful features like resource management, lighting, batch rendering, and more

> ⚠️ Floydia is a **hobby project** and still under active development

Contact me on discord: **vakothebat**

Orbiting cubes, Wavefront model, Plane and the Skybox
![showcase gif](medias/showcase.gif)

Billboard, Cube, Two Crossed Planes and the Skybox
![showcase image](medias/showcase.png)

---

# Features
- **Beginner-Friendly API**: Designed with simplicity in mind, easy to learn for new developers while remaining powerful enough for advanced users
- **2D and 3D development**: Easy-to-use tools for 2D and 3D development:
	+ Basic shapes: `Cube`, `Sprite`, `Billboard`
	+ Wavefront OBJ loading with `.mtl` material support
	+ `PerspectiveCamera` and `OrthoCamera`
- **Lighting System**: Blinn-Phong shading with three light types: Directional, Point and Spot
- **Material System**
	+ Per-object `metallic` and `roughness` properties
	+ Custom GLSL shader support via separable shader pipeline
- **Batch Renderer**: Automatic instanced draw call batching grouped by mesh, shader and texture
- **Frustum Culling**: Automatic per-object visibility testing against the camera frustum
- **AABB**: Per-model and per-submesh axis-aligned bounding boxes
- **Asset Manager**: Automatic caching of meshes, shaders, textures and materials

---

# Libraries used
- [`RGFW`](https://github.com/ColleagueRiley/RGFW) for window and input management
- [`miniaudio`](https://miniaud.io/) for sound handling
- [`stb_image`](https://github.com/nothings/stb) for image loading
- [`stb_truetype`](https://github.com/nothings/stb) for TTF font loading
- [`GLAD`](https://github.com/Dav1dde/glad) for OpenGL loader
- [`glm`](https://github.com/g-truc/glm) for vector and matrix math
- [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader) for Wavefront OBJ loader

> Licenses for all third-party libraries are avaiable in the [`licenses/`](licenses/) directory

## Dependencies
- `GLAD`
- `glm`
- `libxcursor`
- `libxrandr`
- `libxi`
- `mesa` (`libgl1-mesa-dev` on Debian)

No additional headers are required beyond the ones listed above

---

# Example: Rotating Cube
This example creates a single 3D scene with a rotating cube
```cpp

```

---

# Compiling
The library has been tested on **Linux** and **Windows**

## Build Commands
Outputs a static and shared library under `build/`:
```sh
make release
```

With debug flags:
```sh
make debug vars
```

## Debug Macros
| Macro                       | Description                             |
|-----------------------------|-----------------------------------------|
| `FLOYD_DEBUG_RENDERER`      | Logs batch creation and draw call count |
| `FLOYD_DEBUG_MAPPED_BUFFER` | Logs SSBO/UBO resize events             |


## Using the Library
To use the provided [Makefile](examples/Makefile), follow this project structure:
```py
your_project/
├── src/               # Your source files
├── lib/
│   └── floydia/       # Built static/shared library
├── include/
│   ├── floydia/       # Floydia headers
│   └── external/glad/ # GLAD header
│   └── external/glm/  # glm header
```

Then run:
```sh
make
```

---

# Contributing
Floydia is a personal project, but contributions are very welcome!

# LICENSE
This project is licensed under a **modified zlib License**. See the [LICENSE](LICENSE) file for full terms

