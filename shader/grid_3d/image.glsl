float calc_light(vec2 frag_coord)
{
  vec2 pos = frag_coord / iResolution.xy * 2.0 - 1.0;
  
  float z_depth = abs(4.0 / pos.y);
  float x_depth = pos.x * z_depth;
  
  float light = 0.0;
  
  if (pos.y < 0.0) {
    float xd = x_depth;
    float zd = z_depth;
    
    float c = cos(xd);
    float s = sin(zd + iTime * 10.0);
    
    float d = abs(pos.y - 0.1);
    float alpha = 8.0 * max(pow(c * c, 120.0), pow(s * s, 120.0)) * (d * d * d);
    
    light += alpha;
  }
  
  float dx = length(pos * vec2(1.0, 10.0));
  float dy = length(pos * vec2(1.0, 0.8));
  light += (5.0 / dx + 1.0 / dy) * 0.05;
  
  return light;
}


void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  float light = calc_light(frag_coord);
  frag_color = vec4(vec3(1.0, 0.3, 1.0) * light, 1.0);
}
