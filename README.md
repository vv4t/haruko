# Haruko

A toy shadertoy.

## Installation
  > git clone https://github.com/vv4t/haruko.git
  > cd haruko
  > make

## Usage
  Setup up a `.cfg` describing shader configuration. See examples in [shader](https://github.com/vv4t/haruko/tree/main/shader). Then run it with the command 
  > haruko [cfg-file]

## Config Files
In a config file you can set up buffers which either run a shader or reads some disk texture/cubemap.

```
# Loading a plain shader buffer
buffer [buffer name] [shader path];

# Loading a buffer which takes inputs from other buffers
buffer [buffer name] [shader path] {
    iChannel[X] <- [buffer name],
    iChannel[X] <- [buffer name]
    ...
};

# Loading an image
load_image [buffer name] [image path]

# Loading a cubemap
# - This will load "{right,left,up,down,front,back}.{ext}"
# - See shader/skybox for example
load_cubemap [buffer name] [directory path] [image extension]
```

## Shader Files
Shaders are always fragment shaders with the `.glsl` extension. Shader code is written in a `void mainImage(out vec4 fragColor, in vec2 fragCoord)` function. See examples in [shader](https://github.com/vv4t/haruko/tree/main/shader).

Various input variables are built in.

```
# Input buffers defined in the cfg file
uniform sampler2D/samplerCube iChannel[0-3];

# Resolution
# - .x : width of buffer
# - .y : height of buffer
# - .z : size of pixel (only 1.0 for now)
uniform vec3 iResolution;

# Mouse
# - .x : x screen position of mouse
# - .y : y screen position of mouse
# - .z : x clicked position of mouse (negative if mouse button being held down)
# - .w : y clicked position of mouse (negative the frame which the mouse button was clicked)
# See shader/mouse/image.glsl for examples
uniform vec4 iMouse;

# iTime
# - Time elapsed since shader started running (imperfect for now)
uniform float iTime;
```
