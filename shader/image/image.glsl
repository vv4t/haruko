void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  frag_color = texture(iChannel0, frag_coord / iResolution.xy);
}
