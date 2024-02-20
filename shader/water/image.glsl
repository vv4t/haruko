#define MIN_DISTANCE 0.01
#define MAX_DISTANCE 1000.0
#define NUM_STEPS 64

float smin(float a, float b) {
  float k = 0.2;
  float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
  return mix(b, a, h) - k * h * (1.0 - h);
}

float map(vec3 p)
{
  float s1 = length(p - vec3(0.0, cos(iTime), 3.0)) - 1.0;
  float p1 = dot(p, vec3(0.0, 1.0, 0.0)) - (-0.3);
  float h = texture(iChannel1, fract(p.xz * 0.5)).r * 0.005;
  return smin(p1 - h, s1);
}

vec3 map_normal(vec3 p, float d)
{
  float dp = 0.01;
  float dx = map(p + vec3(dp, 0.0, 0.0));
  float dy = map(p + vec3(0.0, dp, 0.0));
  float dz = map(p + vec3(0.0, 0.0, dp));
  return normalize((vec3(dx, dy, dz) - d) / dp);
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
      
      vec3 I = normalize(p - ro);
      vec3 R = reflect(I, n);
      
      return texture(iChannel0, R).rgb * 1.0;
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

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy * 2.0 - 1.0;
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  float ar = iResolution.x / iResolution.y;
  
  vec3 ray = normalize(get_ray(m, uv, ar));
  vec3 diffuse = ray_march(vec3(0.5, 0.0, 0.5), ray);
  
  frag_color = vec4(diffuse, 1.0);
}
