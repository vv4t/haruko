float calc_light(vec2 frag_coord)
{
  vec2 pos = frag_coord * 2.0 - 1.0;
  
  float z_depth = abs(8.0 / pos.y);
  float x_depth = pos.x * z_depth;
  
  float light = 0.0;
  
  if (pos.y < 0.0) {
    float xd = x_depth;
    float zd = z_depth;
    
    float rot = sin(iTime * 3.0) * 0.0;
    
    float xr = xd * cos(rot) - zd * sin(rot);
    float zr = xd * sin(rot) + zd * cos(rot) + iTime * 10.0;
    
    float c = cos(xr);
    float s = sin(zr);
    
    float alpha = 3.0 * (pow(c, 120.0) + pow(s, 120.0));
    
    light += alpha;
  }
  
  float d = length(pos * vec2(1.0, 10.0));
  float d2 = length(pos * vec2(1.0, 0.5));
  light += 0.05 / pow(d, 2.0) + 0.01 / pow(d2, 2.0);
  
  return light;
}


void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  float light = 0.0;
  
  vec2 sample_pos[4] = vec2[4](
    vec2(-0.5, 0.0),
    vec2(+0.5, 0.0),
    vec2(0.0, -0.5),
    vec2(0.0, +0.5)
  );
  
  for (int i = 0; i < 4; i++) {
    light += calc_light(frag_coord + sample_pos[i] * 0.001) / 4.0;
  }
  
  frag_color = vec4(vec3(1.0, 0.3, 1.0) * light, 1.0);
}

