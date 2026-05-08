# UNFINISHED

# Floydia 🪲 v1.0
**Floydia** is a C++ OpenGL 4.5 graphics library designed to simplify 2D and 3D development. It provides an easy-to-use, beginner-friendly API while still offering powerful features like resource management, custom shaders, batch drawing, and more

> ⚠️ Floydia is a **hobby project** and still under active development

Contact me on discord: **vakothebat**

Orbiting cubes, Wavefront model, Plane and the Skybox
![showcase gif](medias/showcase.gif)

Billboard, Cube, Two Crossed Planes and the Skybox
![showcase image](medias/showcase.png)

---

# Features
- **Beginner-Friendly API**: Designed with simplicity in mind, it is easy to learn for new developers while remaining powerful enough for advanced users
- **2D and 3D development**: Easy-to-use tools for 2D and 3D rendering. Includes:
	+ Basic 2D/3D shapes
	+ Material component
- **Memory Management**: Automatic memory handling with a built-in resource manager and batch renderer
- **Built-in systems**
	+ Clock, Timer and Alarm
	+ Wavefront OBJ loading
	+ Frustum Culling
	+ Dynamic light sources
	+ AABB
	+ Sounds
- **Custom Shader Support**: Easily load and apply custom GLSL shaders

---

# Libraries used
- [`RGFW`](https://github.com/ColleagueRiley/RGFW) for window management
- [`miniaudio`](https://miniaud.io/) for sound handling
- [`stb_image`](https://github.com/nothings/stb) for image loading
- [`stb_truetype`](https://github.com/nothings/stb) for TTF font loading
- [`GLAD`](https://github.com/Dav1dde/glad) for OpenGL loader
- [`glm`](https://github.com/g-truc/glm) for vector and matrix math
- [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader) for wavefront OBJ loader

> Licenses for these libraries are in the [`licenses/`](licenses/) directory

## Dependencies
- `GLAD`
- `glm`
- `libxcursor`
- `libxrandr`
- `libxi`
- `mesa` (`libgl1-mesa-dev` on debian)

No other header files are necessary

---

# Example: Rotating Cube
This example creates a single 3D scene with a rotating cube
```cpp

```

---

# Compiling
The library was tested on `Linux` and `Windows` only

## Macros
### Debug
- `FLOYD_DEBUG_RENDERER`: Logs created batches
- `FLOYD_DEBUG_MAPPED_BUFFER`: Logs when a SSBO/UBO are resized

## Command
This will output a static and shared library under `build/`
```sh
make release
```

To compile with **debug** flags:
```sh
make debug
```

## Using the Library
To use the provided [Makefile](examples/Makefile), follow this structure:
```py
your_project/
├── src/               # Your .cpp files
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

