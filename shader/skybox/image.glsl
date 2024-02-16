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

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy * 2.0 - 1.0;
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  float aspect_ratio = iResolution.x / iResolution.y;
  
  vec3 ray = calc_camera_ray(m, uv, aspect_ratio);
  
  frag_color = texture(iChannel0, ray);
}
