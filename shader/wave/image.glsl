void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 uv = frag_coord / iResolution.xy;
  
  if (texture(iChannel1, uv).r > 0.0) {
    frag_color = vec4(1.0);
    return;
  }
  
  float h = texture(iChannel0, uv).r;
  
  float x = 0.0;
  float y = 0.0;
  
  if (h < 0.0) {
    x = -h;
  } else {
    y = h;
  }
  
  vec3 a = vec3(0.0, 0.0, 1.0);
  vec3 b = vec3(0.0, 1.0, 1.0);
  vec3 c = vec3(0.0, 1.0, 1.0);
  
  frag_color = vec4(mix(a * x, b * y, 0.5) + mix(x, y, 0.5) * c, 1.0);
}
