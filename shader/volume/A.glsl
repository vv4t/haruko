
float rand(vec3 co) {
  return fract(sin(dot(co, vec3(12.9898, 78.233, 94.123))) * 43758.5453);
}

void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  float depth = 8.0;
  
  vec2 uv = frag_coord / iResolution.xy;
  vec3 p = vec3(fract(uv * depth), dot(floor(uv * depth), vec2(1.0, depth)) / pow(depth, 2.0));
  
  vec3 light = vec3(0.0);
  light += cos(p.x * 8.0 + iTime * 4.0) * 0.01;
  
  frag_color = vec4(light, 1.0);
}
