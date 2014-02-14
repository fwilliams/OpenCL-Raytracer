#ifndef __OPENCL_VERSION__

#define kernel
#define global
#define constant
#define local

typedef struct float2 {
  float x, y;
} float2;

typedef struct float3 {
  float x, y, z;
  float2 xy, xz, yx, yz, zx, zy;
} float3;

typedef struct float4 {
  float x, y, z, w;
  float2 xy, yx;
  float3 xyz, xzy, yxz, yzx, zxy, zyx;
} float4;

int get_global_id(int);

float3 normalize(float3);

float3 matrixVectorMultiply(float*, float3*);

float dot(float3, float3);

float sqrt(float);

float length(float3);

float4 clamp(float4, float, float);

float max(float, float);

#endif
