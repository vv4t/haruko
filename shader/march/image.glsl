#define MIN_DISTANCE 0.01
#define MAX_DISTANCE 1000.0
#define NUM_STEPS 128

vec3 rot_xz(vec3 v, float c, float s);
vec3 get_ray(vec2 m, vec2 uv, float ar);
float map(vec3 p);
vec3 ray_march(vec3 ro, vec3 rd);

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy * 2.0 - 1.0;
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  float ar = iResolution.x / iResolution.y;
  
  vec3 ray = normalize(get_ray(m, uv, ar));
  vec3 diffuse = ray_march(vec3(0.0), ray);
  
  frag_color = vec4(diffuse, 1.0);
}

float smin(float a, float b) {
  float k = 0.2;
  float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
  return mix(b, a, h) - k * h * (1.0 - h);
}

float luminance(vec3 col) {
  return (0.2126*col.r + 0.7152*col.g + 0.0722*col.b);
}

float map(vec3 p)
{
  // float s1 = length(p - vec3(0.0, 0.0, 1.5)) - 1.0;
  float p1 = dot(p, vec3(0.0, 1.0, 0.0)) - (-0.3);
  
  float h = luminance(texture(iChannel1, p.xz).rgb) * 0.05;
  
  return p1 - h;
}

vec3 map_normal(vec3 p, float d)
{
  float dp = 0.01;
  float dx = map(p + vec3(dp, 0.0, 0.0));
  float dy = map(p + vec3(0.0, dp, 0.0));
  float dz = map(p + vec3(0.0, 0.0, dp));
  return normalize((vec3(dx, dy, dz) - d) / dp);
}

vec2 map_uv(vec3 p, vec3 n)
{
  vec3 v = abs(n);
  
  if (v.x > v.y) {
    if (v.z > v.x) {
      return vec2(p.x, p.y);
    } else {
      return vec2(p.z, p.y);
    }
  } else {
    if (v.z > v.y) {
      return vec2(p.x, p.y);
    } else {
      return vec2(p.x, p.z);
    }
  }
}

float rand(vec2 co){
  return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 ray_march(vec3 ro, vec3 rd)
{
  vec3 p = ro;
  float td = 0.0;
  
  for (int i = 0; i < NUM_STEPS; i++) {
    p = ro + rd * td;
    
    float d = map(p);
    
    if (d < MIN_DISTANCE) {
      vec3 n = map_normal(p, d);
      vec2 uv = map_uv(p, n);
      
      vec3 I = normalize(p);
      vec3 R = reflect(I, n);
      
      vec3 ld = p - vec3(2.0, 2.0, 0.0);
      
      return vec3(dot(n, normalize(-ld))) + texture(iChannel0, R).rgb * 0.2;
    }
    
    if (td > MAX_DISTANCE) {
      break;
    }
    
    td += d;
  }
  
  return texture(iChannel0, rd).rgb;
}

vec3 get_ray(vec2 m, vec2 uv, float ar)
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
  
  return TBN * vec3(uv * vec2(ar, 1.0), 1.0);
}
