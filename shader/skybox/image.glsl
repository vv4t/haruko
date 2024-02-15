void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  vec2 m = (iMouse.xy / iResolution.xy * 2.0 - 1.0) * 4.0;
  
  vec3 forward = vec3(-sin(m.x), 0.0, cos(m.x));
  vec3 side = vec3(cos(m.x), 0.0, sin(m.x));
  vec3 up = vec3(0.0, 1.0, 0.0);
  mat3 TBN = mat3(side, up, forward);
  
  vec3 ray = TBN * vec3(frag_coord / iResolution.xy * 2.0 - 1.0, 1.0);
  frag_color = texture(iChannel0, ray);
}
