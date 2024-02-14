void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  float alpha = (1.0 + iMouse.z * 5.0) / length(frag_coord - iMouse.xy);
  frag_color = vec4(vec3(1.0, 0.3, 1.0) * alpha, 1.0);
}
