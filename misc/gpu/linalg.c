int loc(int v0, int v1, int *neighbors_start, int *neighbors) {
  int l = neighbors_start[v0];
  while(neighbors[l] != v1) l++;
  return l;
}

//  0 1 2
//  3 4 5
//  6 7 8
double det3_f64(double * a){
  return a[0] * (a[4]*a[8]-a[5]*a[7]) 
       - a[1] * (a[3]*a[8]-a[5]*a[6])
       + a[2] * (a[3]*a[7]-a[4]*a[6]);
}
float det3_f32(float * a){
  return a[0] * (a[4]*a[8]-a[5]*a[7]) 
       - a[1] * (a[3]*a[8]-a[5]*a[6])
       + a[2] * (a[3]*a[7]-a[4]*a[6]);
}

void inv3_f64(double * a, double * inv){
  double cof00 = a[4]*a[8]-a[5]*a[7];
  double cof01 =-a[3]*a[8]+a[5]*a[6];
  double cof02 = a[3]*a[7]-a[4]*a[6];
  
  double cof10 =-a[1]*a[8]+a[2]*a[7];
  double cof11 = a[0]*a[8]-a[2]*a[6];
  double cof12 =-a[0]*a[7]+a[1]*a[6];

  double cof20 = a[1]*a[5]-a[2]*a[4];
  double cof21 =-a[0]*a[5]+a[2]*a[3];
  double cof22 = a[0]*a[4]-a[1]*a[3];

  double determ = a[0] * cof00 + a[1] * cof01 + a[2]*cof02;
  
  determ = 1.0/determ;
  inv[0] = cof00 * determ;
  inv[1] = cof10 * determ;
  inv[2] = cof20 * determ;

  inv[3] = cof01 * determ;
  inv[4] = cof11 * determ;
  inv[5] = cof21 * determ;

  inv[6] = cof02 * determ;
  inv[7] = cof12 * determ;
  inv[8] = cof22 * determ;
  
}
void inv3_f32(float * a, float * inv){
  float cof00 = a[4]*a[8]-a[5]*a[7];
  float cof01 =-a[3]*a[8]+a[5]*a[6];
  float cof02 = a[3]*a[7]-a[4]*a[6];
  
  float cof10 =-a[1]*a[8]+a[2]*a[7];
  float cof11 = a[0]*a[8]-a[2]*a[6];
  float cof12 =-a[0]*a[7]+a[1]*a[6];

  float cof20 = a[1]*a[5]-a[2]*a[4];
  float cof21 =-a[0]*a[5]+a[2]*a[3];
  float cof22 = a[0]*a[4]-a[1]*a[3];

  float determ = a[0] * cof00 + a[1] * cof01 + a[2]*cof02;
  
  determ = 1.0/determ;
  inv[0] = cof00 * determ;
  inv[1] = cof10 * determ;
  inv[2] = cof20 * determ;

  inv[3] = cof01 * determ;
  inv[4] = cof11 * determ;
  inv[5] = cof21 * determ;

  inv[6] = cof02 * determ;
  inv[7] = cof12 * determ;
  inv[8] = cof22 * determ;
}

float __nv_sqrtf(float v);
float complexNorm_f32(float r, float i) {
  return __nv_sqrtf(r*r+i*i);
}
