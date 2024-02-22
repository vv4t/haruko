#define MIN_DISTANCE 0.01
#define MAX_DISTANCE 1000.0
#define NUM_STEPS 64

float cube(vec3 p, vec3 o, vec3 s)
{
  vec3 d = abs(p - (o + s)) - s;
  return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float map(vec3 p)
{
  vec3 q = vec3(-1.0, -1.0, -1.0) * 1.0;
  float c1 = cube(p, q, vec3(1.0));
  return c1;
}

vec3 illuminate(vec3 p, vec3 ro)
{
  vec3 q = vec3(-1.0, -1.0, -1.0) * 1.0;
  vec3 a = (p - q) * 0.5;
  vec3 rd = normalize(p - ro) * 0.01;
  
  vec3 light = vec3(0.0);
  
  float depth = 8.0;
  
  for (int i = 0; i < 128; i++) {
    a += rd;
    
    float z = floor(a.z * depth * depth);
    
    vec2 uv = vec2(a.xy) / depth;
    uv.x += floor(mod(z, depth)) / depth;
    uv.y += floor(z / depth) / depth;
    
    light += texture(iChannel0, uv).rgb;
    
    if (max(a.x, max(a.y, a.z)) > 1.0 + MIN_DISTANCE || min(a.x, min(a.y, a.z)) < -MIN_DISTANCE) break;
  }
  
  return light;
}

vec3 raymarch(vec3 ro, vec3 rd)
{
  vec3 p = ro;
  float td = 0.0;
  
  for (int i = 0; i < NUM_STEPS; i++) {
    p = ro + rd * td;
    
    float d = map(p);
    
    if (d < MIN_DISTANCE) return illuminate(p, ro) + texture(iChannel1, rd).rgb;
    if (td > MAX_DISTANCE) break;
    
    td += d;
  }
  
  return texture(iChannel1, rd).rgb;
}

mat3 look_at(vec3 at, vec3 up)
{
  vec3 z = normalize(at);
  vec3 x = normalize(cross(up, z));
  vec3 y = normalize(cross(z, x));
  
  return mat3(x, y, z);
}

mat3 free_look(vec2 m)
{
  float cx = cos(-m.x);
  float sx = sin(-m.x);
  
  float cy = cos(-m.y);
  float sy = sin(-m.y);
  
  vec3 forward = vec3(0.0, -sy, cy);
  vec3 up = vec3(0.0, cy, sy);
  vec3 side = vec3(1.0, 0.0, 0.0);
  
  mat3 rot = mat3(
    cx, 0.0, sx,
    0.0, 1.0, 0.0,
    -sx, 0.0, cx
  );
  
  mat3 TBN = rot * mat3(side, up, forward);
  
  return TBN;
}

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy * 2.0 - 1.0;
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  float ar = iResolution.x / iResolution.y;
  
  // mat3 view = look_at(m);
  
  float cx = cos(-m.x);
  float sx = sin(-m.x);
  
  float cy = cos(-m.y);
  float sy = sin(-m.y);
  
  mat3 rot = mat3(
    cx, 0.0, sx,
    0.0, 1.0, 0.0,
    -sx, 0.0, cx
  );
  
  vec3 up = rot * vec3(0.0, -cy, sy);
  
  vec3 from = rot * vec3(0.0, sy, cy) * 4.0;
  vec3 at = vec3(0.0, 0.0, 0.0);
  mat3 view = look_at(at - from, up);
  
  vec3 ray = normalize(view * vec3(uv * vec2(ar, 1.0), 1.0));
  vec3 diffuse = raymarch(from, ray);
  
  frag_color = vec4(diffuse, 1.0);
}

