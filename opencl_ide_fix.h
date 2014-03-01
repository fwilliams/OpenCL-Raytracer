#ifndef __OPENCL_VERSION__

#define kernel
#define global
#define constant
#define local
#define write_only
#define read_only

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
	float2 xy, yx;
	float3 xyz, xzy, yxz, yzx, zxy, zyx;
} float4;

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

#endif
