void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  if (texture(iChannel2, frag_coord / iResolution.xy).r > 0.0) {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }
  
  float h = texture(iChannel0, frag_coord / iResolution.xy).r + 0.5;
  frag_color = vec4(vec3(h), 1.0);
}
