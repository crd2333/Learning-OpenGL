#import "@templates/report:0.1.0": *

#show: project.with(
  title: "《计算机图形学》实验报告",
  title_2: "Assignment 4: Solar System",
  date: (2024, 10, 30),
  cover_style: "",
  lang: "zh",
  header: "type3",
  show_toc: false
)

#note(caption: [Source File Structure (briefly description)])[
  #tree-list(root: "Solar_System")[
    - resources
      - shader_f.fs
      - shader_v.vs
      - textures
        - `*.jpg` for earth, mars, moon, sun
    - main.cpp  #h(1fr) main entry, and the render loop
    - camera.h  #h(1fr) header from LearnOpenGL, for camera control
    - shader.h  #h(1fr) header from LearnOpenGL, for shader control
    - config.h #h(1fr) configurations for window, time control, and stars/planets/seltites
    - sphere.hpp #h(1fr) `Pattern` class to initialize and `Sphere` class producing vertices
    - texture.hpp #h(1fr) `Texture` class to load and bind textures
  ]
]

#warning(caption: "Credits")[
  - Textures from #link("https://www.solarsystemscope.com/textures/")
  - Some header files from #link("https://learnopengl-cn.github.io/") (with a little modification)
  - Others are implemented by myself purely
]

- in `main.cpp`
  ```cpp
  /* include files */

  /* initialize camera and time */

  /* functions for interaction */

  int main {
    /* initialize window and OpenGL */

    /* initialize one shader(and settings), textures and one sphere */

    while (!glfwWindowShouldClose(window)) {
      /* process input, clear window and update camera */

      // render sun
      // render earth
      // render moon
      // render mars
    }
    /* release resources */
  }
  ```

- `shader.h` and `camera.h` are from LearnOpenGL. Only add the `UP` and `DOWN` direction for camera control in `camera.h`

- `config.h` is a header file for configurations, including
  - window size, time control
    - I treat unit of `glfwGetTime()` as hours, so there is an global speeding up ratio
  - stars/planets/seltites settings, including *radius(scale)*, *distance*, *RotationAngle*, *RotationSpeed*, *revolution speed*

- `sphere.hpp`
  - One class `Pattern` to *initialize* and bind *VAO*, *VBO*, *EBO* for any pattern, and *designed to*
    - *adapt* to *different shaders*, such as
      + A vertex with color in respective RGB format
      + A vertex with color in uniform RGB format
      + A vertex with texture coordinate
    - *adapt* to cases where *vertices* have been *created or not* (create later)
    - *adapt* to cases where *using indices or not*
    - *Convenient*: once created, just bind shader (and texture) and call `draw()` for rendering
  - One class `Sphere` inherit from `Pattern` to construct vertices and indices
    - according to the *radius*, *Longitude and Latitude Resolution* (they are adjustable)
    - $u in [0, 180]$, $v in [0, 360]$, create two triangles at each position, like the followiing
    #fig("sphere.png", caption: none, width: 67%)

- `texture.hpp`: one class to load textures and return the texture ID, nothing special

#hline()

- How to navigate in the space?
  - `W`, `A`, `S`, `D` for moving *forward*, *left*, *backward*, *right*
  - `SPACE` for moving *up*, `LEFT SHIFT` + `SPACE` for moving *down*
  - use *mouse* to control the *view direction*
- compile commandline:
  - `g++ -std=c++17 -Wall -Wextra -g -Iinclude -o output/main.exe src/main.cpp -Llib -lglad -lglfw3dll -llibassimp -lOpengl32 -lglu32`
  - `./output/main.exe` with `glfw3.dll`, `libassimp-5.dll` in the same directory
  - Maybe need some specific lib or include requirements, and dynamic link libraries


- How is the *rotation* and *revolution* implemented
  - For each star/planet/seltite, use a `glm::mat4` to store the model matrix
  - *explicitly* compute the *revolution* by `sin`, `cos` and `dis`, use `translate` to move
  - use `rotate` to *tilt* the star/planet/seltite *on its axis* according to the angle, and then `rotate` *around the axis*
  - use `scale` to scale the star/planet/seltite
  - For all the stars/planets/seltites, they are rendered by changing *only the model matrix* and *texture*. The sphere is *reused*

- For more details, please refer to the *source code* and the *comments*

#success(caption: "Check")[
  - [x] –2 suns, 2+ planets, 1+ satellite
    - Sun, Earth, Mars, Moon
    - an additional smaller sun in a casual position
  - [x] Planets orbit around the sun
    - Earth, Mars around the Sun
  - [x] Satellites orbit around its planet
    - Moon around the Earth
  - [x] Trajectories are not co-planar
    - each sphere rotate around its axis
  - [x] Navigation in the system (3D viewing)
]

- Result like this

#fig("result.png", caption: none)