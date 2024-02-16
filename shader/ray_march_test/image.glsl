vec3 rotated_xz(vec3 v, float c, float s)
{
  return vec3(
    v.x * c - v.z * s,
    v.y,
    v.x * s + v.z * c
  );
}

vec3 calc_camera_ray(vec2 m, vec2 uv, float aspect_ratio)
{
  float cx = cos(-m.x);
  float sx = sin(-m.x);
  
  float cy = cos(-m.y);
  float sy = sin(-m.y);
  
  vec3 forward = vec3(0.0, -sy, cy);
  vec3 up = vec3(0.0, cy, sy);
  vec3 side = vec3(1.0, 0.0, 0.0);
  
  forward = rotated_xz(forward, cx, sx);
  up = rotated_xz(up, cx, sx);
  side = rotated_xz(side, cx, sx);
  
  mat3 TBN = mat3(side, up, forward);
  
  return TBN * vec3(uv * vec2(aspect_ratio, 1.0), 1.0);
}

float sphere_distance(vec3 p, vec3 o, float r)
{
  return length(p - o) - r;
}

vec3 ray_march(vec3 ro, vec3 rd)
{
  vec3 sphere_pos = vec3(0.0, 0.0, 1.0);
  float sphere_radius = 0.6;
  
  float MIN_DISTANCE = 0.01;
  float MAX_DISTANCE = 1000.0;
  int NUM_STEPS = 32;
  
  vec3 light_dir = normalize(vec3(1.0, -1.0, 1.0));
  
  vec3 pos = ro;
  float total_distance = 0.0;
  
  for (int i = 0; i < NUM_STEPS; i++) {
    pos = ro + rd * total_distance;
    
    float distance = sphere_distance(pos, sphere_pos, sphere_radius);
    
    if (distance < MIN_DISTANCE) {
      vec3 normal = normalize(pos - sphere_pos);
      
      vec3 I = normalize(pos - ro);
      vec3 R = reflect(I, normalize(normal));
      
      float diffuse = dot(normal, -light_dir);
      
      float light = diffuse;
      
      return vec3(light) + texture(iChannel0, R).rgb;
    }
    
    if (distance > MAX_DISTANCE) {
      break;
    }
    
    total_distance += distance;
  }
  
  return texture(iChannel0, rd).rgb;
}

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy * 2.0 - 1.0;
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  float aspect_ratio = iResolution.x / iResolution.y;
  
  vec3 ray = normalize(calc_camera_ray(m, uv, aspect_ratio));
  vec3 diffuse = ray_march(vec3(0.0), ray);
  
  frag_color = vec4(diffuse, 1.0);
}
