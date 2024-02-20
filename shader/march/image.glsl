#define MIN_DISTANCE 0.01
#define MAX_DISTANCE 1000.0
#define NUM_STEPS 64

float smin(float a, float b) {
  float k = 0.2;
  float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
  return mix(b, a, h) - k * h * (1.0 - h);
}

float sphere(vec3 p, vec3 o, float r)
{
  return length(p - o) - r;
}

float cube(vec3 p, vec3 o, vec3 s)
{
  vec3 d = abs(p - o) - s;
  return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float plane(vec3 p, vec3 n, float d)
{
  return dot(p, n) - d;
}

float map(vec3 p)
{
  float s1 = sphere(p, vec3(0.0, cos(iTime), 1.5), 0.5);
  float c1 = cube(p, vec3(0.0, sin(iTime), 1.5), vec3(0.5, 0.5, 0.5));
  float p1 = plane(p, vec3(0.0, 1.0, 0.0), -0.3);
  
  return smin(smin(c1, s1), p1);
}

vec3 map_normal(vec3 p)
{
  float dp = 0.01;
  float d = map(p);
  float dx = map(p + vec3(dp, 0.0, 0.0));
  float dy = map(p + vec3(0.0, dp, 0.0));
  float dz = map(p + vec3(0.0, 0.0, dp));
  return normalize((vec3(dx, dy, dz) - d) / dp);
}

float shadow(vec3 pt, vec3 rd)
{
  vec3 p = pt;
  float td = MIN_DISTANCE * 3.0;
  float kd = 1.0;
  
  for (int i = 0; i < NUM_STEPS && kd > 0.01; i++) {
    p = pt + rd * td;
    
    float d = map(p);
    
    if (d < MIN_DISTANCE) kd = 0.0;
    else kd = min(kd, 16.0 * d / td);
    
    if (td > MAX_DISTANCE) break;
    
    td += d;
  }
  
  return kd;
}

vec3 illuminate(vec3 p, vec3 ro)
{
  vec3 lp = vec3(1.0, 0.5, 3.0);
  vec3 ld = lp - p;
  
  vec3 n = map_normal(p);
  
  vec3 I = normalize(p - ro);
  vec3 R = reflect(I, n);
  
  float alpha = clamp(dot(n, ld), 0.0, 1.0);
  float attenuation = 1.0 / dot(ld, ld);
  float light = alpha * attenuation * shadow(p, normalize(ld));
  
  // return vec3(1.0) * light;
  return vec3(1.0) * light + texture(iChannel0, R).rgb * 0.1;
}

vec3 ray_march(vec3 ro, vec3 rd)
{
  vec3 p = ro;
  float td = 0.0;
  
  for (int i = 0; i < NUM_STEPS; i++) {
    p = ro + rd * td;
    
    float d = map(p);
    
    if (d < MIN_DISTANCE) return illuminate(p, ro);
    if (td > MAX_DISTANCE) break;
    
    td += d;
  }
  
  return texture(iChannel0, reflect(normalize(p - ro), map_normal(p))).rgb * 0.1;
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

