#ifndef __OPENCL_VERSION__

#define kernel
#define global
#define constant
#define local
#define write_only
#define read_only

#define MAX_RENDER_DISTANCE 10000000.0
#define NUM_TRIANGLES 1
#define NUM_SPHERES 1
#define NUM_MATERIALS 1
#define NUM_LIGHTS 1
#define CLK_NORMALIZED_COORDS_TRUE 0
#define CLK_ADDRESS_REPEAT 0
#define CLK_FILTER_LINEAR 0
#define CLK_FILTER_NEAREST 0

typedef int sampler_t;

typedef struct int2 {
	int x, y;
} int2;

typedef struct int3 {
	int x, y;
	int2 xy, xz, yx, yz, zx, zy;
} int3;

typedef struct int4 {
	int x, y, z, w;
	int2 xy, yx;
	int3 xyz, xzy, yxz, yzx, zxy, zyx;
} int4;

typedef struct float2 {
	float x, y;
} float2;

typedef struct float3 {
	float x, y, z;
	float2 xy, xz, yx, yz, zx, zy;
} float3;

typedef struct float4 {
	float x, y, z, w;
	float2 xy, xz, xw, yx, yz, yw, zx, zy, zw, wx, wy, wz;
	float3 xyz, xzy, xzw, xyw, yxz, yzx, zxy, zyx;
} float4;

typedef struct float16 {
	float s0, s1, s2, s3, s4, s5, s6, s7,
	s8, s9, sa, sA, sb, sB, sc, sC, sd, sD, se, sE, sf, sF;
	float2 s01, s02, s03, s04, s05, s06, s07, s08, s09, s0a, s0b, s0c, s0d, s0e, s0f;
	float3 s012, s013, s014;
	float4 s0123, s4567, s89ab, s89AB, scdef, sCDEF;
} float16;

typedef float* image2d_t;

int get_global_id(int);

int get_global_size(int);

float3 normalize(float3);

float dot(float3, float3);

float3 cross(float3, float3);

float sqrt(float);

float length(float3);

float4 clamp(float4, float, float);
float3 clamp(float3, float, float);

float max(float, float);

void write_imagef(image2d_t image, int2 coord, float4 color);
float4 read_imagef(image2d_t image, sampler_t sampler, float2 coord);

#endif
