<img src="https://static.vecteezy.com/system/resources/thumbnails/014/612/285/small/comic-light-rays-background-png.png" align="right" width=150 style="margin:10px"/>

# <a href="https://www.opengl.org"><img src="https://upload.wikimedia.org/wikipedia/commons/e/e9/Opengl-logo.svg" align="center" width=150 style="margin:0px"/></a> rt


<em>Praise the Sun ☀️</em>

# Objective ❓

The goal is to implement a RayTracer taking advantage of GPU paralelism with the help of compute shaders in OpenGL.

# Dependencies 📦

- <a href="https://www.glfw.org/"><img src="https://www.glfw.org/img/favicon/favicon-196x196.png" width=30 align="center"/> **GLFW**</a>
- <a href="https://github.com/g-truc/glm"><img src="https://upload.wikimedia.org/wikipedia/commons/5/5b/GLM_logo.png" align="center" width=50/> **GLM**</a>
- <a href="https://github.com/nothings/stb">**STB**</a>
- <a href="https://glad.dav1d.de/"> **GLAD**</a>

# Progress ⏳

![Progress](/assets//progress.png)

# Primitives

- [ ] Plane 🖼️
- [ ] Sphere 🌏
- [ ] Cylinder 🥫
- [ ] Cone 🍧

# Methodology

La BRDF NO es una FUNCION que diga como deben rebotar los rayos de nuestro raytracer, la BRDF lo unico que sabe es que, si de mis rayos aleatorios, uno de ellos va en cierta direccion (acorde al modelo del material) entonces ese rayo tiene menos energia. SIN EMBARGO, para hacer los calculos mas rapidos, se usa una funcion SAMPLE que lanza rayos PRECISAMENTE en la direccion en la que sabemos que va a haber mas contribucion al color.

# Export

Generated images are exported in the following format:

`t[compute time]_exp[exposure]_iso[iso]_k[sensor constant]_n[aperture size].hdr`

- Time is in seconds.
- All values are multiplied by 100.

# HDR viewer

- [OpenHDR Viewer](https://viewer.openhdr.org/)

# TODO

- [ ] Intereseccion RAYO - QUAD
- [ ] Implementar distintas BRDF