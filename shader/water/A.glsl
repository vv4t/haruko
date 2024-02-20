void mainImage(out vec4 frag_color, in vec2 frag_coord)
{
  ivec2 uv = ivec2(frag_coord);
  
  vec2 p = texelFetch(iChannel0, uv, 0).xy;
  
  float u = p.r;
  float u_t = p.g;
  
  float du_dx_1 = u - texelFetch(iChannel0, ivec2(uv.x - 1, uv.y), 0).x;
  float du_dy_1 = u - texelFetch(iChannel0, ivec2(uv.x, uv.y - 1), 0).x;
  
  float du_dx_2 = texelFetch(iChannel0, ivec2(uv.x + 1, uv.y), 0).x - u;
  float du_dy_2 = texelFetch(iChannel0, ivec2(uv.x, uv.y + 1), 0).x - u;
  
  float d2u_dx2 = du_dx_2 - du_dx_1;
  float d2u_dy2 = du_dy_2 - du_dy_1;
  
  float d2u_dt2 = d2u_dx2 + d2u_dy2;
  
  float c = 0.4;
  
  float u_0 = u * 2.0 - u_t + c * d2u_dt2;
  
  frag_color = vec4(u_0, u, 0.0, 1.0);
}
