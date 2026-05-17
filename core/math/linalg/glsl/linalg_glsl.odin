// `GLSL`-like mathematics library plus numerous other utility procedures.
package math_linalg_glsl

import "base:builtin"
import "base:intrinsics"

TAU :: 6.28318530717958647692528676655900576
PI  :: 3.14159265358979323846264338327950288
E   :: 2.71828182845904523536
τ   :: TAU
π   :: PI
e   :: E

SQRT_TWO   :: 1.41421356237309504880168872420969808
SQRT_THREE :: 1.73205080756887729352744634150587236
SQRT_FIVE  :: 2.23606797749978969640917366873127623

LN2  :: 0.693147180559945309417232121458176568
LN10 :: 2.30258509299404568401799145468436421

F32_EPSILON :: 1e-7
F64_EPSILON :: 1e-15

// Odin matrices are stored internally as Column-Major, which matches OpenGL/GLSL by default
mat2 :: matrix[2, 2]f32
mat3 :: matrix[3, 3]f32
mat4 :: matrix[4, 4]f32
mat2x2 :: mat2
mat3x3 :: mat3
mat4x4 :: mat4

// IMPORTANT NOTE: These data types are "backwards" in normal mathematical terms
// but they match how GLSL and OpenGL defines them in name
// Odin: matrix[R, C]f32 
// GLSL: matCxR
mat3x2 :: matrix[2, 3]f32
mat4x2 :: matrix[2, 4]f32
mat2x3 :: matrix[3, 2]f32
mat4x3 :: matrix[3, 4]f32
mat2x4 :: matrix[4, 2]f32
mat3x4 :: matrix[4, 3]f32

vec2 :: [2]f32
vec3 :: [3]f32
vec4 :: [4]f32

ivec2 :: [2]i32
ivec3 :: [3]i32
ivec4 :: [4]i32

uvec2 :: [2]u32
uvec3 :: [3]u32
uvec4 :: [4]u32

bvec2 :: [2]bool
bvec3 :: [3]bool
bvec4 :: [4]bool

quat :: quaternion128

// Double Precision (f64) Floating Point Types 

dmat2 :: matrix[2, 2]f64
dmat3 :: matrix[3, 3]f64
dmat4 :: matrix[4, 4]f64
dmat2x2 :: dmat2
dmat3x3 :: dmat3
dmat4x4 :: dmat4

dmat3x2 :: matrix[2, 3]f64
dmat4x2 :: matrix[2, 4]f64
dmat2x3 :: matrix[3, 2]f64
dmat4x3 :: matrix[3, 4]f64
dmat2x4 :: matrix[4, 2]f64
dmat3x4 :: matrix[4, 3]f64

dvec2 :: [2]f64
dvec3 :: [3]f64
dvec4 :: [4]f64

dquat :: quaternion256

cos :: proc{
	cos_f32,
	cos_f64,
	cos_vec2,
	cos_vec3,
	cos_vec4,
	cos_dvec2,
	cos_dvec3,
	cos_dvec4,
}
@(require_results) cos_vec2  :: proc "c" (x: vec2) -> vec2 { return {cos(x.x), cos(x.y)} }
@(require_results) cos_vec3  :: proc "c" (x: vec3) -> vec3 { return {cos(x.x), cos(x.y), cos(x.z)} }
@(require_results) cos_vec4  :: proc "c" (x: vec4) -> vec4 { return {cos(x.x), cos(x.y), cos(x.z), cos(x.w)} }
@(require_results) cos_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {cos(x.x), cos(x.y)} }
@(require_results) cos_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {cos(x.x), cos(x.y), cos(x.z)} }
@(require_results) cos_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {cos(x.x), cos(x.y), cos(x.z), cos(x.w)} }

sin :: proc{
	sin_f32,
	sin_f64,
	sin_vec2,
	sin_vec3,
	sin_vec4,
	sin_dvec2,
	sin_dvec3,
	sin_dvec4,
}
@(require_results) sin_vec2  :: proc "c" (x: vec2) -> vec2 { return {sin(x.x), sin(x.y)} }
@(require_results) sin_vec3  :: proc "c" (x: vec3) -> vec3 { return {sin(x.x), sin(x.y), sin(x.z)} }
@(require_results) sin_vec4  :: proc "c" (x: vec4) -> vec4 { return {sin(x.x), sin(x.y), sin(x.z), sin(x.w)} }
@(require_results) sin_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {sin(x.x), sin(x.y)} }
@(require_results) sin_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {sin(x.x), sin(x.y), sin(x.z)} }
@(require_results) sin_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {sin(x.x), sin(x.y), sin(x.z), sin(x.w)} }

tan :: proc{
	tan_f32,
	tan_f64,
	tan_vec2,
	tan_vec3,
	tan_vec4,
	tan_dvec2,
	tan_dvec3,
	tan_dvec4,
}
@(require_results) tan_vec2  :: proc "c" (x: vec2) -> vec2 { return {tan(x.x), tan(x.y)} }
@(require_results) tan_vec3  :: proc "c" (x: vec3) -> vec3 { return {tan(x.x), tan(x.y), tan(x.z)} }
@(require_results) tan_vec4  :: proc "c" (x: vec4) -> vec4 { return {tan(x.x), tan(x.y), tan(x.z), tan(x.w)} }
@(require_results) tan_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {tan(x.x), tan(x.y)} }
@(require_results) tan_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {tan(x.x), tan(x.y), tan(x.z)} }
@(require_results) tan_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {tan(x.x), tan(x.y), tan(x.z), tan(x.w)} }

acos :: proc{
	acos_f32,
	acos_f64,
	acos_vec2,
	acos_vec3,
	acos_vec4,
	acos_dvec2,
	acos_dvec3,
	acos_dvec4,
}
@(require_results) acos_vec2  :: proc "c" (x: vec2) -> vec2 { return {acos(x.x), acos(x.y)} }
@(require_results) acos_vec3  :: proc "c" (x: vec3) -> vec3 { return {acos(x.x), acos(x.y), acos(x.z)} }
@(require_results) acos_vec4  :: proc "c" (x: vec4) -> vec4 { return {acos(x.x), acos(x.y), acos(x.z), acos(x.w)} }
@(require_results) acos_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {acos(x.x), acos(x.y)} }
@(require_results) acos_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {acos(x.x), acos(x.y), acos(x.z)} }
@(require_results) acos_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {acos(x.x), acos(x.y), acos(x.z), acos(x.w)} }

asin :: proc{
	asin_f32,
	asin_f64,
	asin_vec2,
	asin_vec3,
	asin_vec4,
	asin_dvec2,
	asin_dvec3,
	asin_dvec4,
}
@(require_results) asin_vec2  :: proc "c" (x: vec2) -> vec2 { return {asin(x.x), asin(x.y)} }
@(require_results) asin_vec3  :: proc "c" (x: vec3) -> vec3 { return {asin(x.x), asin(x.y), asin(x.z)} }
@(require_results) asin_vec4  :: proc "c" (x: vec4) -> vec4 { return {asin(x.x), asin(x.y), asin(x.z), asin(x.w)} }
@(require_results) asin_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {asin(x.x), asin(x.y)} }
@(require_results) asin_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {asin(x.x), asin(x.y), asin(x.z)} }
@(require_results) asin_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {asin(x.x), asin(x.y), asin(x.z), asin(x.w)} }

atan :: proc{
	atan_f32,
	atan_f64,
	atan_vec2,
	atan_vec3,
	atan_vec4,
	atan_dvec2,
	atan_dvec3,
	atan_dvec4,
	atan2_f32,
	atan2_f64,
	atan2_vec2,
	atan2_vec3,
	atan2_vec4,
	atan2_dvec2,
	atan2_dvec3,
	atan2_dvec4,
}
@(require_results) atan_vec2  :: proc "c" (x: vec2) -> vec2 { return {atan(x.x), atan(x.y)} }
@(require_results) atan_vec3  :: proc "c" (x: vec3) -> vec3 { return {atan(x.x), atan(x.y), atan(x.z)} }
@(require_results) atan_vec4  :: proc "c" (x: vec4) -> vec4 { return {atan(x.x), atan(x.y), atan(x.z), atan(x.w)} }
@(require_results) atan_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {atan(x.x), atan(x.y)} }
@(require_results) atan_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {atan(x.x), atan(x.y), atan(x.z)} }
@(require_results) atan_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {atan(x.x), atan(x.y), atan(x.z), atan(x.w)} }

atan2 :: proc{
	atan2_f32,
	atan2_f64,
	atan2_vec2,
	atan2_vec3,
	atan2_vec4,
	atan2_dvec2,
	atan2_dvec3,
	atan2_dvec4,
}
@(require_results) atan2_vec2 :: proc "c" (y, x: vec2) -> vec2 { return {atan2(y.x, x.x), atan2(y.y, x.y)} }
@(require_results) atan2_vec3 :: proc "c" (y, x: vec3) -> vec3 { return {atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z)} }
@(require_results) atan2_vec4 :: proc "c" (y, x: vec4) -> vec4 { return {atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z), atan2(y.w, x.w)} }
@(require_results) atan2_dvec2 :: proc "c" (y, x: dvec2) -> dvec2 { return {atan2(y.x, x.x), atan2(y.y, x.y)} }
@(require_results) atan2_dvec3 :: proc "c" (y, x: dvec3) -> dvec3 { return {atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z)} }
@(require_results) atan2_dvec4 :: proc "c" (y, x: dvec4) -> dvec4 { return {atan2(y.x, x.x), atan2(y.y, x.y), atan2(y.z, x.z), atan2(y.w, x.w)} }



cosh :: proc{
	cosh_f32,
	cosh_f64,
	cosh_vec2,
	cosh_vec3,
	cosh_vec4,
	cosh_dvec2,
	cosh_dvec3,
	cosh_dvec4,
}
@(require_results) cosh_vec2 :: proc "c" (x: vec2) -> vec2 { return {cosh(x.x), cosh(x.y)} }
@(require_results) cosh_vec3 :: proc "c" (x: vec3) -> vec3 { return {cosh(x.x), cosh(x.y), cosh(x.z)} }
@(require_results) cosh_vec4 :: proc "c" (x: vec4) -> vec4 { return {cosh(x.x), cosh(x.y), cosh(x.z), cosh(x.w)} }
@(require_results) cosh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {cosh(x.x), cosh(x.y)} }
@(require_results) cosh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {cosh(x.x), cosh(x.y), cosh(x.z)} }
@(require_results) cosh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {cosh(x.x), cosh(x.y), cosh(x.z), cosh(x.w)} }


sinh :: proc{
	sinh_f32,
	sinh_f64,
	sinh_vec2,
	sinh_vec3,
	sinh_vec4,
	sinh_dvec2,
	sinh_dvec3,
	sinh_dvec4,
}
@(require_results) sinh_vec2 :: proc "c" (x: vec2) -> vec2 { return {sinh(x.x), sinh(x.y)} }
@(require_results) sinh_vec3 :: proc "c" (x: vec3) -> vec3 { return {sinh(x.x), sinh(x.y), sinh(x.z)} }
@(require_results) sinh_vec4 :: proc "c" (x: vec4) -> vec4 { return {sinh(x.x), sinh(x.y), sinh(x.z), sinh(x.w)} }
@(require_results) sinh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {sinh(x.x), sinh(x.y)} }
@(require_results) sinh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {sinh(x.x), sinh(x.y), sinh(x.z)} }
@(require_results) sinh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {sinh(x.x), sinh(x.y), sinh(x.z), sinh(x.w)} }

tanh :: proc{
	tanh_f32,
	tanh_f64,
	tanh_vec2,
	tanh_vec3,
	tanh_vec4,
	tanh_dvec2,
	tanh_dvec3,
	tanh_dvec4,
}
@(require_results) tanh_vec2 :: proc "c" (x: vec2) -> vec2 { return {tanh(x.x), tanh(x.y)} }
@(require_results) tanh_vec3 :: proc "c" (x: vec3) -> vec3 { return {tanh(x.x), tanh(x.y), tanh(x.z)} }
@(require_results) tanh_vec4 :: proc "c" (x: vec4) -> vec4 { return {tanh(x.x), tanh(x.y), tanh(x.z), tanh(x.w)} }
@(require_results) tanh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {tanh(x.x), tanh(x.y)} }
@(require_results) tanh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {tanh(x.x), tanh(x.y), tanh(x.z)} }
@(require_results) tanh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {tanh(x.x), tanh(x.y), tanh(x.z), tanh(x.w)} }

acosh :: proc{
	acosh_f32,
	acosh_f64,
	acosh_vec2,
	acosh_vec3,
	acosh_vec4,
	acosh_dvec2,
	acosh_dvec3,
	acosh_dvec4,
}
@(require_results) acosh_vec2 :: proc "c" (x: vec2) -> vec2 { return {acosh(x.x), acosh(x.y)} }
@(require_results) acosh_vec3 :: proc "c" (x: vec3) -> vec3 { return {acosh(x.x), acosh(x.y), acosh(x.z)} }
@(require_results) acosh_vec4 :: proc "c" (x: vec4) -> vec4 { return {acosh(x.x), acosh(x.y), acosh(x.z), acosh(x.w)} }
@(require_results) acosh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {acosh(x.x), acosh(x.y)} }
@(require_results) acosh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {acosh(x.x), acosh(x.y), acosh(x.z)} }
@(require_results) acosh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {acosh(x.x), acosh(x.y), acosh(x.z), acosh(x.w)} }

asinh :: proc{
	asinh_f32,
	asinh_f64,
	asinh_vec2,
	asinh_vec3,
	asinh_vec4,
	asinh_dvec2,
	asinh_dvec3,
	asinh_dvec4,
}
@(require_results) asinh_vec2 :: proc "c" (x: vec2) -> vec2 { return {asinh(x.x), asinh(x.y)} }
@(require_results) asinh_vec3 :: proc "c" (x: vec3) -> vec3 { return {asinh(x.x), asinh(x.y), asinh(x.z)} }
@(require_results) asinh_vec4 :: proc "c" (x: vec4) -> vec4 { return {asinh(x.x), asinh(x.y), asinh(x.z), asinh(x.w)} }
@(require_results) asinh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {asinh(x.x), asinh(x.y)} }
@(require_results) asinh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {asinh(x.x), asinh(x.y), asinh(x.z)} }
@(require_results) asinh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {asinh(x.x), asinh(x.y), asinh(x.z), asinh(x.w)} }

atanh :: proc{
	atanh_f32,
	atanh_f64,
	atanh_vec2,
	atanh_vec3,
	atanh_vec4,
	atanh_dvec2,
	atanh_dvec3,
	atanh_dvec4,
}
@(require_results) atanh_vec2 :: proc "c" (x: vec2) -> vec2 { return {atanh(x.x), atanh(x.y)} }
@(require_results) atanh_vec3 :: proc "c" (x: vec3) -> vec3 { return {atanh(x.x), atanh(x.y), atanh(x.z)} }
@(require_results) atanh_vec4 :: proc "c" (x: vec4) -> vec4 { return {atanh(x.x), atanh(x.y), atanh(x.z), atanh(x.w)} }
@(require_results) atanh_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {atanh(x.x), atanh(x.y)} }
@(require_results) atanh_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {atanh(x.x), atanh(x.y), atanh(x.z)} }
@(require_results) atanh_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {atanh(x.x), atanh(x.y), atanh(x.z), atanh(x.w)} }

sqrt :: proc{
	sqrt_f32,
	sqrt_f64,
	sqrt_vec2,
	sqrt_vec3,
	sqrt_vec4,
	sqrt_dvec2,
	sqrt_dvec3,
	sqrt_dvec4,
}
@(require_results) sqrt_vec2 :: proc "c" (x: vec2) -> vec2 { return {sqrt(x.x), sqrt(x.y)} }
@(require_results) sqrt_vec3 :: proc "c" (x: vec3) -> vec3 { return {sqrt(x.x), sqrt(x.y), sqrt(x.z)} }
@(require_results) sqrt_vec4 :: proc "c" (x: vec4) -> vec4 { return {sqrt(x.x), sqrt(x.y), sqrt(x.z), sqrt(x.w)} }
@(require_results) sqrt_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {sqrt(x.x), sqrt(x.y)} }
@(require_results) sqrt_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {sqrt(x.x), sqrt(x.y), sqrt(x.z)} }
@(require_results) sqrt_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {sqrt(x.x), sqrt(x.y), sqrt(x.z), sqrt(x.w)} }

rsqrt :: inversesqrt
inversesqrt :: proc{
	inversesqrt_f32,
	inversesqrt_f64,
	inversesqrt_vec2,
	inversesqrt_vec3,
	inversesqrt_vec4,
	inversesqrt_dvec2,
	inversesqrt_dvec3,
	inversesqrt_dvec4,
}
@(require_results) inversesqrt_vec2 :: proc "c" (x: vec2) -> vec2 { return {inversesqrt(x.x), inversesqrt(x.y)} }
@(require_results) inversesqrt_vec3 :: proc "c" (x: vec3) -> vec3 { return {inversesqrt(x.x), inversesqrt(x.y), inversesqrt(x.z)} }
@(require_results) inversesqrt_vec4 :: proc "c" (x: vec4) -> vec4 { return {inversesqrt(x.x), inversesqrt(x.y), inversesqrt(x.z), inversesqrt(x.w)} }
@(require_results) inversesqrt_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {inversesqrt(x.x), inversesqrt(x.y)} }
@(require_results) inversesqrt_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {inversesqrt(x.x), inversesqrt(x.y), inversesqrt(x.z)} }
@(require_results) inversesqrt_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {inversesqrt(x.x), inversesqrt(x.y), inversesqrt(x.z), inversesqrt(x.w)} }


pow :: proc{
	pow_f32,
	pow_f64,
	pow_vec2,
	pow_vec3,
	pow_vec4,
	pow_dvec2,
	pow_dvec3,
	pow_dvec4,
}
@(require_results) pow_vec2 :: proc "c" (x, y: vec2) -> vec2 { return {pow(x.x, y.x), pow(x.y, y.y)} }
@(require_results) pow_vec3 :: proc "c" (x, y: vec3) -> vec3 { return {pow(x.x, y.x), pow(x.y, y.y), pow(x.z, y.z)} }
@(require_results) pow_vec4 :: proc "c" (x, y: vec4) -> vec4 { return {pow(x.x, y.x), pow(x.y, y.y), pow(x.z, y.z), pow(x.w, y.w)} }
@(require_results) pow_dvec2 :: proc "c" (x, y: dvec2) -> dvec2 { return {pow(x.x, y.x), pow(x.y, y.y)} }
@(require_results) pow_dvec3 :: proc "c" (x, y: dvec3) -> dvec3 { return {pow(x.x, y.x), pow(x.y, y.y), pow(x.z, y.z)} }
@(require_results) pow_dvec4 :: proc "c" (x, y: dvec4) -> dvec4 { return {pow(x.x, y.x), pow(x.y, y.y), pow(x.z, y.z), pow(x.w, y.w)} }



exp :: proc{
	exp_f32,
	exp_f64,
	exp_vec2,
	exp_vec3,
	exp_vec4,
	exp_dvec2,
	exp_dvec3,
	exp_dvec4,
}
@(require_results) exp_vec2 :: proc "c" (x: vec2) -> vec2 { return {exp(x.x), exp(x.y)} }
@(require_results) exp_vec3 :: proc "c" (x: vec3) -> vec3 { return {exp(x.x), exp(x.y), exp(x.z)} }
@(require_results) exp_vec4 :: proc "c" (x: vec4) -> vec4 { return {exp(x.x), exp(x.y), exp(x.z), exp(x.w)} }
@(require_results) exp_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {exp(x.x), exp(x.y)} }
@(require_results) exp_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {exp(x.x), exp(x.y), exp(x.z)} }
@(require_results) exp_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {exp(x.x), exp(x.y), exp(x.z), exp(x.w)} }



log :: proc{
	log_f32,
	log_f64,
	log_vec2,
	log_vec3,
	log_vec4,
	log_dvec2,
	log_dvec3,
	log_dvec4,
}
@(require_results) log_vec2 :: proc "c" (x: vec2) -> vec2 { return {log(x.x), log(x.y)} }
@(require_results) log_vec3 :: proc "c" (x: vec3) -> vec3 { return {log(x.x), log(x.y), log(x.z)} }
@(require_results) log_vec4 :: proc "c" (x: vec4) -> vec4 { return {log(x.x), log(x.y), log(x.z), log(x.w)} }
@(require_results) log_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {log(x.x), log(x.y)} }
@(require_results) log_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {log(x.x), log(x.y), log(x.z)} }
@(require_results) log_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {log(x.x), log(x.y), log(x.z), log(x.w)} }



exp2 :: proc{
	exp2_f32,
	exp2_f64,
	exp2_vec2,
	exp2_vec3,
	exp2_vec4,
	exp2_dvec2,
	exp2_dvec3,
	exp2_dvec4,
}
@(require_results) exp2_vec2 :: proc "c" (x: vec2) -> vec2 { return {exp2(x.x), exp2(x.y)} }
@(require_results) exp2_vec3 :: proc "c" (x: vec3) -> vec3 { return {exp2(x.x), exp2(x.y), exp2(x.z)} }
@(require_results) exp2_vec4 :: proc "c" (x: vec4) -> vec4 { return {exp2(x.x), exp2(x.y), exp2(x.z), exp2(x.w)} }
@(require_results) exp2_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {exp2(x.x), exp2(x.y)} }
@(require_results) exp2_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {exp2(x.x), exp2(x.y), exp2(x.z)} }
@(require_results) exp2_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {exp2(x.x), exp2(x.y), exp2(x.z), exp2(x.w)} }


sign :: proc{
	sign_i32,
	sign_u32,
	sign_f32,
	sign_f64,
	sign_vec2,
	sign_vec3,
	sign_vec4,
	sign_dvec2,
	sign_dvec3,
	sign_dvec4,
	sign_ivec2,
	sign_ivec3,
	sign_ivec4,
	sign_uvec2,
	sign_uvec3,
	sign_uvec4,
}
@(require_results) sign_i32 :: proc "c" (x: i32) -> i32 { return -1 if x < 0 else +1 if x > 0 else 0 }
@(require_results) sign_u32 :: proc "c" (x: u32) -> u32 { return +1 if x > 0 else 0 }
@(require_results) sign_vec2 :: proc "c" (x: vec2) -> vec2 { return {sign(x.x), sign(x.y)} }
@(require_results) sign_vec3 :: proc "c" (x: vec3) -> vec3 { return {sign(x.x), sign(x.y), sign(x.z)} }
@(require_results) sign_vec4 :: proc "c" (x: vec4) -> vec4 { return {sign(x.x), sign(x.y), sign(x.z), sign(x.w)} }
@(require_results) sign_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {sign(x.x), sign(x.y)} }
@(require_results) sign_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {sign(x.x), sign(x.y), sign(x.z)} }
@(require_results) sign_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {sign(x.x), sign(x.y), sign(x.z), sign(x.w)} }
@(require_results) sign_ivec2 :: proc "c" (x: ivec2) -> ivec2 { return {sign(x.x), sign(x.y)} }
@(require_results) sign_ivec3 :: proc "c" (x: ivec3) -> ivec3 { return {sign(x.x), sign(x.y), sign(x.z)} }
@(require_results) sign_ivec4 :: proc "c" (x: ivec4) -> ivec4 { return {sign(x.x), sign(x.y), sign(x.z), sign(x.w)} }
@(require_results) sign_uvec2 :: proc "c" (x: uvec2) -> uvec2 { return {sign(x.x), sign(x.y)} }
@(require_results) sign_uvec3 :: proc "c" (x: uvec3) -> uvec3 { return {sign(x.x), sign(x.y), sign(x.z)} }
@(require_results) sign_uvec4 :: proc "c" (x: uvec4) -> uvec4 { return {sign(x.x), sign(x.y), sign(x.z), sign(x.w)} }

floor :: proc{
	floor_f32,
	floor_f64,
	floor_vec2,
	floor_vec3,
	floor_vec4,
	floor_dvec2,
	floor_dvec3,
	floor_dvec4,
}
@(require_results) floor_vec2 :: proc "c" (x: vec2) -> vec2 { return {floor(x.x), floor(x.y)} }
@(require_results) floor_vec3 :: proc "c" (x: vec3) -> vec3 { return {floor(x.x), floor(x.y), floor(x.z)} }
@(require_results) floor_vec4 :: proc "c" (x: vec4) -> vec4 { return {floor(x.x), floor(x.y), floor(x.z), floor(x.w)} }
@(require_results) floor_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {floor(x.x), floor(x.y)} }
@(require_results) floor_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {floor(x.x), floor(x.y), floor(x.z)} }
@(require_results) floor_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {floor(x.x), floor(x.y), floor(x.z), floor(x.w)} }

trunc :: proc{
	trunc_f32,
	trunc_f64,
	trunc_vec2,
	trunc_vec3,
	trunc_vec4,
	trunc_dvec2,
	trunc_dvec3,
	trunc_dvec4,
}
@(require_results) trunc_vec2 :: proc "c" (x: vec2) -> vec2 { return {trunc(x.x), trunc(x.y)} }
@(require_results) trunc_vec3 :: proc "c" (x: vec3) -> vec3 { return {trunc(x.x), trunc(x.y), trunc(x.z)} }
@(require_results) trunc_vec4 :: proc "c" (x: vec4) -> vec4 { return {trunc(x.x), trunc(x.y), trunc(x.z), trunc(x.w)} }
@(require_results) trunc_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {trunc(x.x), trunc(x.y)} }
@(require_results) trunc_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {trunc(x.x), trunc(x.y), trunc(x.z)} }
@(require_results) trunc_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {trunc(x.x), trunc(x.y), trunc(x.z), trunc(x.w)} }


round :: proc{
	round_f32,
	round_f64,
	round_vec2,
	round_vec3,
	round_vec4,
	round_dvec2,
	round_dvec3,
	round_dvec4,
}
@(require_results) round_vec2 :: proc "c" (x: vec2) -> vec2 { return {round(x.x), round(x.y)} }
@(require_results) round_vec3 :: proc "c" (x: vec3) -> vec3 { return {round(x.x), round(x.y), round(x.z)} }
@(require_results) round_vec4 :: proc "c" (x: vec4) -> vec4 { return {round(x.x), round(x.y), round(x.z), round(x.w)} }
@(require_results) round_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {round(x.x), round(x.y)} }
@(require_results) round_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {round(x.x), round(x.y), round(x.z)} }
@(require_results) round_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {round(x.x), round(x.y), round(x.z), round(x.w)} }


ceil :: proc{
	ceil_f32,
	ceil_f64,
	ceil_vec2,
	ceil_vec3,
	ceil_vec4,
	ceil_dvec2,
	ceil_dvec3,
	ceil_dvec4,
}
@(require_results) ceil_vec2 :: proc "c" (x: vec2) -> vec2 { return {ceil(x.x), ceil(x.y)} }
@(require_results) ceil_vec3 :: proc "c" (x: vec3) -> vec3 { return {ceil(x.x), ceil(x.y), ceil(x.z)} }
@(require_results) ceil_vec4 :: proc "c" (x: vec4) -> vec4 { return {ceil(x.x), ceil(x.y), ceil(x.z), ceil(x.w)} }
@(require_results) ceil_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {ceil(x.x), ceil(x.y)} }
@(require_results) ceil_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {ceil(x.x), ceil(x.y), ceil(x.z)} }
@(require_results) ceil_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {ceil(x.x), ceil(x.y), ceil(x.z), ceil(x.w)} }


mod :: proc{
	mod_f32,
	mod_f64,
	mod_vec2,
	mod_vec3,
	mod_vec4,
	mod_dvec2,
	mod_dvec3,
	mod_dvec4,
}
@(require_results) mod_vec2 :: proc "c" (x, y: vec2) -> vec2 { return {mod(x.x, y.x), mod(x.y, y.y)} }
@(require_results) mod_vec3 :: proc "c" (x, y: vec3) -> vec3 { return {mod(x.x, y.x), mod(x.y, y.y), mod(x.z, y.z)} }
@(require_results) mod_vec4 :: proc "c" (x, y: vec4) -> vec4 { return {mod(x.x, y.x), mod(x.y, y.y), mod(x.z, y.z), mod(x.w, y.w)} }
@(require_results) mod_dvec2 :: proc "c" (x, y: dvec2) -> dvec2 { return {mod(x.x, y.x), mod(x.y, y.y)} }
@(require_results) mod_dvec3 :: proc "c" (x, y: dvec3) -> dvec3 { return {mod(x.x, y.x), mod(x.y, y.y), mod(x.z, y.z)} }
@(require_results) mod_dvec4 :: proc "c" (x, y: dvec4) -> dvec4 { return {mod(x.x, y.x), mod(x.y, y.y), mod(x.z, y.z), mod(x.w, y.w)} }


fract :: proc{
	fract_f32,
	fract_f64,
	fract_vec2,
	fract_vec3,
	fract_vec4,
	fract_dvec2,
	fract_dvec3,
	fract_dvec4,
}
@(require_results) fract_vec2 :: proc "c" (x: vec2) -> vec2 { return {fract(x.x), fract(x.y)} }
@(require_results) fract_vec3 :: proc "c" (x: vec3) -> vec3 { return {fract(x.x), fract(x.y), fract(x.z)} }
@(require_results) fract_vec4 :: proc "c" (x: vec4) -> vec4 { return {fract(x.x), fract(x.y), fract(x.z), fract(x.w)} }
@(require_results) fract_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return {fract(x.x), fract(x.y)} }
@(require_results) fract_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return {fract(x.x), fract(x.y), fract(x.z)} }
@(require_results) fract_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return {fract(x.x), fract(x.y), fract(x.z), fract(x.w)} }



radians :: proc{
	radians_f32,
	radians_f64,
	radians_vec2,
	radians_vec3,
	radians_vec4,
	radians_dvec2,
	radians_dvec3,
	radians_dvec4,
}
@(require_results) radians_f32  :: proc "c" (degrees: f32)  -> f32  { return degrees * TAU / 360.0 }
@(require_results) radians_f64  :: proc "c" (degrees: f64)  -> f64  { return degrees * TAU / 360.0 }
@(require_results) radians_vec2 :: proc "c" (degrees: vec2) -> vec2 { return degrees * TAU / 360.0 }
@(require_results) radians_vec3 :: proc "c" (degrees: vec3) -> vec3 { return degrees * TAU / 360.0 }
@(require_results) radians_vec4 :: proc "c" (degrees: vec4) -> vec4 { return degrees * TAU / 360.0 }
@(require_results) radians_dvec2 :: proc "c" (degrees: dvec2) -> dvec2 { return degrees * TAU / 360.0 }
@(require_results) radians_dvec3 :: proc "c" (degrees: dvec3) -> dvec3 { return degrees * TAU / 360.0 }
@(require_results) radians_dvec4 :: proc "c" (degrees: dvec4) -> dvec4 { return degrees * TAU / 360.0 }


degrees :: proc{
	degrees_f32,
	degrees_f64,
	degrees_vec2,
	degrees_vec3,
	degrees_vec4,
	degrees_dvec2,
	degrees_dvec3,
	degrees_dvec4,
}
@(require_results) degrees_f32  :: proc "c" (radians: f32)  -> f32  { return radians * 360.0 / TAU }
@(require_results) degrees_f64  :: proc "c" (radians: f64)  -> f64  { return radians * 360.0 / TAU }
@(require_results) degrees_vec2 :: proc "c" (radians: vec2) -> vec2 { return radians * 360.0 / TAU }
@(require_results) degrees_vec3 :: proc "c" (radians: vec3) -> vec3 { return radians * 360.0 / TAU }
@(require_results) degrees_vec4 :: proc "c" (radians: vec4) -> vec4 { return radians * 360.0 / TAU }
@(require_results) degrees_dvec2 :: proc "c" (radians: dvec2) -> dvec2 { return radians * 360.0 / TAU }
@(require_results) degrees_dvec3 :: proc "c" (radians: dvec3) -> dvec3 { return radians * 360.0 / TAU }
@(require_results) degrees_dvec4 :: proc "c" (radians: dvec4) -> dvec4 { return radians * 360.0 / TAU }

min :: builtin.min
max :: builtin.max
clamp :: builtin.clamp

@(private)
@(require_results)
saturate_scalar :: proc "c" (v: $T) -> T where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return builtin.clamp(v, 0, 1)
}
@(private)
@(require_results)
saturate_array :: proc "c" (v: $A/[$N]$T) -> A where intrinsics.type_is_numeric(T) {
	return builtin.clamp(v, 0, 1)
}
saturate :: proc{saturate_scalar, saturate_array}

@(private)
@(require_results)
mix_scalar :: proc "c" (x, y, t: $T) -> T where intrinsics.type_is_float(T), !intrinsics.type_is_array(T) {
	return x*(1-t) + y*t
}
@(private)
@(require_results)
mix_array :: proc "c" (x, y, t: $A/[$N]$T) -> A where intrinsics.type_is_float(T) {
	return x*(1-t) + y*t
}
mix :: proc{mix_scalar, mix_array}
lerp :: mix

@(private)
@(require_results)
step_scalar :: proc "c" (edge, x: $T) -> T where intrinsics.type_is_float(T), !intrinsics.type_is_array(T) {
	return 0 if x < edge else 1
}
@(private)
@(require_results)
step_array :: proc "c" (edge, x: $A/[$N]$T) -> A where intrinsics.type_is_float(T) {
	out: A
	for i in 0..<N {
		out[i] = step_scalar(edge[i], x[i])
	}
	return out
}
step :: proc{step_scalar, step_array}

@(private)
@(require_results)
smoothstep_scalar :: proc "c" (edge0, edge1, x: $T) -> T where intrinsics.type_is_float(T), !intrinsics.type_is_array(T) {
	y := clamp(((x-edge0) / (edge1-edge0)), 0, 1)
	return y * y * (3 - 2*y)
}
@(private)
@(require_results)
smoothstep_array :: proc "c" (edge0, edge1, x: $A/[$N]$T) -> A where intrinsics.type_is_float(T) {
	y := clamp(((x-edge0) / (edge1-edge0)), 0, 1)
	return y * y * (3 - 2*y)
}
smoothstep :: proc{smoothstep_scalar, smoothstep_array}

@(private)
@(require_results)
abs_numeric_scalar :: proc "c" (x: $T) -> T where intrinsics.type_is_numeric(T), !intrinsics.type_is_unsigned(T), !intrinsics.type_is_array(T) {
	return builtin.abs(x)
}
@(private)
@(require_results)
abs_numeric_array :: proc "c" (x: $A/[$N]$T) -> A where intrinsics.type_is_numeric(T), !intrinsics.type_is_unsigned(T) {
	return builtin.abs(x)
}
@(private)
@(require_results)
abs_unsigned_scalar :: proc "c" (x: $T) -> T where intrinsics.type_is_unsigned(T), !intrinsics.type_is_array(T) {
	return x
}
@(private)
@(require_results)
abs_unsigned_array :: proc "c" (x: $A/[$N]$T) -> A where intrinsics.type_is_unsigned(T) {
	return x
}
abs :: proc{
	abs_numeric_scalar,
	abs_numeric_array,
	abs_unsigned_scalar,
	abs_unsigned_array,
}

@(private)
@(require_results)
dot_scalar :: proc "c" (a, b: $T) -> T where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T), !intrinsics.type_is_quaternion(T) {
	return a * b
}
@(private)
@(require_results)
dot_array :: proc "c" (a, b: $A/[$N]$T) -> T where intrinsics.type_is_numeric(T) {
	out: T
	for i in 0..<N {
		out += a[i] * b[i]
	}
	return out
}
@(private)
@(require_results)
dot_quaternion128 :: proc "c" (a, b: quat) -> f32 {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w
}
@(private)
@(require_results)
dot_quaternion256 :: proc "c" (a, b: dquat) -> f64 {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w
}
dot :: proc{dot_scalar, dot_array, dot_quaternion128, dot_quaternion256}

@(private)
@(require_results)
length_scalar :: proc "c" (x: $T) -> T where intrinsics.type_is_float(T), !intrinsics.type_is_array(T) {
	return builtin.abs(x)
}
@(private)
@(require_results)
length_array :: proc "c" (x: $A/[$N]$T) -> T where intrinsics.type_is_float(T) {
	return sqrt(dot(x, x))
}
@(private)
@(require_results)
length_quaternion128 :: proc "c" (x: quat) -> f32 {
	return sqrt(dot(x, x))
}
@(private)
@(require_results)
length_quaternion256 :: proc "c" (x: dquat) -> f64 {
	return sqrt(dot(x, x))
}
length :: proc{length_scalar, length_array, length_quaternion128, length_quaternion256}


@(require_results) distance :: proc "c" (x, y: $T) -> T { return length(y-x) }

cross :: proc{
	cross_vec3,
	cross_dvec3,
	cross_ivec3,
}

@(require_results) cross_vec3 :: proc "c" (a, b: vec3) -> (c: vec3) {
	c.x = a.y*b.z - b.y*a.z
	c.y = a.z*b.x - b.z*a.x
	c.z = a.x*b.y - b.x*a.y
	return
}
@(require_results) cross_dvec3 :: proc "c" (a, b: dvec3) -> (c: dvec3) {
	c.x = a.y*b.z - b.y*a.z
	c.y = a.z*b.x - b.z*a.x
	c.z = a.x*b.y - b.x*a.y
	return
}
@(require_results) cross_ivec3 :: proc "c" (a, b: ivec3) -> (c: ivec3) {
	c.x = a.y*b.z - b.y*a.z
	c.y = a.z*b.x - b.z*a.x
	c.z = a.x*b.y - b.x*a.y
	return
}

normalize :: proc{
	normalize_f32,
	normalize_f64,
	normalize_vec2,
	normalize_vec3,
	normalize_vec4,
	normalize_dvec2,
	normalize_dvec3,
	normalize_dvec4,
	normalize_quat,
	normalize_dquat,
}
@(require_results) normalize_f32  :: proc "c" (x: f32)  -> f32  { return 1.0 }
@(require_results) normalize_f64  :: proc "c" (x: f64)  -> f64  { return 1.0 }
@(require_results) normalize_vec2 :: proc "c" (x: vec2) -> vec2 { return x / length(x) }
@(require_results) normalize_vec3 :: proc "c" (x: vec3) -> vec3 { return x / length(x) }
@(require_results) normalize_vec4 :: proc "c" (x: vec4) -> vec4 { return x / length(x) }
@(require_results) normalize_dvec2 :: proc "c" (x: dvec2) -> dvec2 { return x / length(x) }
@(require_results) normalize_dvec3 :: proc "c" (x: dvec3) -> dvec3 { return x / length(x) }
@(require_results) normalize_dvec4 :: proc "c" (x: dvec4) -> dvec4 { return x / length(x) }
@(require_results) normalize_quat :: proc "c" (x: quat) -> quat { return x / quat(length(x)) }
@(require_results) normalize_dquat :: proc "c" (x: dquat) -> dquat { return x / dquat(length(x)) }


faceForward :: proc{
	faceForward_f32,
	faceForward_f64,
	faceForward_vec2,
	faceForward_vec3,
	faceForward_vec4,
	faceForward_dvec2,
	faceForward_dvec3,
	faceForward_dvec4,
}
@(require_results) faceForward_f32  :: proc "c" (N, I, Nref: f32)  -> f32  { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_f64  :: proc "c" (N, I, Nref: f64)  -> f64  { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_vec2 :: proc "c" (N, I, Nref: vec2) -> vec2 { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_vec3 :: proc "c" (N, I, Nref: vec3) -> vec3 { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_vec4 :: proc "c" (N, I, Nref: vec4) -> vec4 { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_dvec2 :: proc "c" (N, I, Nref: dvec2) -> dvec2 { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_dvec3 :: proc "c" (N, I, Nref: dvec3) -> dvec3 { return N if dot(I, Nref) < 0 else -N }
@(require_results) faceForward_dvec4 :: proc "c" (N, I, Nref: dvec4) -> dvec4 { return N if dot(I, Nref) < 0 else -N }


reflect :: proc{
	reflect_f32,
	reflect_f64,
	reflect_vec2,
	reflect_vec3,
	reflect_vec4,
	reflect_dvec2,
	reflect_dvec3,
	reflect_dvec4,
}
@(require_results) reflect_f32  :: proc "c" (I, N: f32)  -> f32  { return I - 2*N*dot(N, I) }
@(require_results) reflect_f64  :: proc "c" (I, N: f64)  -> f64  { return I - 2*N*dot(N, I) }
@(require_results) reflect_vec2 :: proc "c" (I, N: vec2) -> vec2 { return I - 2*N*dot(N, I) }
@(require_results) reflect_vec3 :: proc "c" (I, N: vec3) -> vec3 { return I - 2*N*dot(N, I) }
@(require_results) reflect_vec4 :: proc "c" (I, N: vec4) -> vec4 { return I - 2*N*dot(N, I) }
@(require_results) reflect_dvec2 :: proc "c" (I, N: dvec2) -> dvec2 { return I - 2*N*dot(N, I) }
@(require_results) reflect_dvec3 :: proc "c" (I, N: dvec3) -> dvec3 { return I - 2*N*dot(N, I) }
@(require_results) reflect_dvec4 :: proc "c" (I, N: dvec4) -> dvec4 { return I - 2*N*dot(N, I) }




refract :: proc{
	refract_f32,
	refract_f64,
	refract_vec2,
	refract_vec3,
	refract_vec4,
	refract_dvec2,
	refract_dvec3,
	refract_dvec4,
}
@(require_results) refract_f32  :: proc "c" (i, n, eta: f32) -> f32 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * f32(i32(cost2 > 0))
}
@(require_results) refract_f64  :: proc "c" (i, n, eta: f64) -> f64 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * f64(i32(cost2 > 0))
}
@(require_results) refract_vec2  :: proc "c" (i, n, eta: vec2) -> vec2 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * vec2{f32(i32(cost2.x > 0)), f32(i32(cost2.y > 0))}
}
@(require_results) refract_vec3  :: proc "c" (i, n, eta: vec3) -> vec3 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * vec3{f32(i32(cost2.x > 0)), f32(i32(cost2.y > 0)), f32(i32(cost2.z > 0))}
}
@(require_results) refract_vec4  :: proc "c" (i, n, eta: vec4) -> vec4 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * vec4{f32(i32(cost2.x > 0)), f32(i32(cost2.y > 0)), f32(i32(cost2.z > 0)), f32(i32(cost2.w > 0))}
}
@(require_results) refract_dvec2  :: proc "c" (i, n, eta: dvec2) -> dvec2 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * dvec2{f64(i32(cost2.x > 0)), f64(i32(cost2.y > 0))}
}
@(require_results) refract_dvec3  :: proc "c" (i, n, eta: dvec3) -> dvec3 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * dvec3{f64(i32(cost2.x > 0)), f64(i32(cost2.y > 0)), f64(i32(cost2.z > 0))}
}
@(require_results) refract_dvec4  :: proc "c" (i, n, eta: dvec4) -> dvec4 {
	cosi := dot(-i, n)
	cost2 := 1 - eta*eta*(1 - cosi*cosi)
	t := eta*i + ((eta*cosi - sqrt(abs(cost2))) * n)
	return t * dvec4{f64(i32(cost2.x > 0)), f64(i32(cost2.y > 0)), f64(i32(cost2.z > 0)), f64(i32(cost2.w > 0))}
}

scalarTripleProduct :: proc{
	scalarTripleProduct_vec3,
	scalarTripleProduct_dvec3,
	scalarTripleProduct_ivec3,
}
@(require_results) scalarTripleProduct_vec3 :: proc "c" (a, b, c: vec3) -> f32  { return dot(a, cross(b, c)) }
@(require_results) scalarTripleProduct_dvec3 :: proc "c" (a, b, c: dvec3) -> f64  { return dot(a, cross(b, c)) }
@(require_results) scalarTripleProduct_ivec3 :: proc "c" (a, b, c: ivec3) -> i32  { return dot(a, cross(b, c)) }

vectorTripleProduct :: proc {
	vectorTripleProduct_vec3,
	vectorTripleProduct_dvec3,
	vectorTripleProduct_ivec3,	
}
@(require_results) vectorTripleProduct_vec3 :: proc "c" (a, b, c: vec3) -> vec3 { return cross(a, cross(b, c)) }
@(require_results) vectorTripleProduct_dvec3 :: proc "c" (a, b, c: dvec3) -> dvec3 { return cross(a, cross(b, c)) }
@(require_results) vectorTripleProduct_ivec3 :: proc "c" (a, b, c: ivec3) -> ivec3 { return cross(a, cross(b, c)) }


// Vector Relational Procedures

@(private)
@(require_results)
less_than_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a < b
}
@(private)
@(require_results)
less_than_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a < b
}
lessThan :: proc{less_than_scalar, less_than_array}

@(private)
@(require_results)
less_than_equal_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a <= b
}
@(private)
@(require_results)
less_than_equal_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a <= b
}
lessThanEqual :: proc{less_than_equal_scalar, less_than_equal_array}

@(private)
@(require_results)
greater_than_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a > b
}
@(private)
@(require_results)
greater_than_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a > b
}
greaterThan :: proc{greater_than_scalar, greater_than_array}

@(private)
@(require_results)
greater_than_equal_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a >= b
}
@(private)
@(require_results)
greater_than_equal_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a >= b
}
greaterThanEqual :: proc{greater_than_equal_scalar, greater_than_equal_array}

@(private)
@(require_results)
equal_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a == b
}
@(private)
@(require_results)
equal_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a == b
}
equal :: proc{equal_scalar, equal_array}

@(private)
@(require_results)
not_equal_scalar :: proc "c" (a, b: $T) -> bool where intrinsics.type_is_numeric(T), !intrinsics.type_is_array(T) {
	return a != b
}
@(private)
@(require_results)
not_equal_array :: proc "c" (a, b: $A/[$N]$T) -> [N]bool where intrinsics.type_is_numeric(T) {
	return a != b
}
notEqual :: proc{not_equal_scalar, not_equal_array}

any :: proc{
	any_bool,
	any_bvec2,
	any_bvec3,
	any_bvec4,
}
@(require_results) any_bool  :: proc "c" (v: bool) -> bool  { return v }
@(require_results) any_bvec2 :: proc "c" (v: bvec2) -> bool { return v.x || v.y }
@(require_results) any_bvec3 :: proc "c" (v: bvec3) -> bool { return v.x || v.y || v.z }
@(require_results) any_bvec4 :: proc "c" (v: bvec4) -> bool { return v.x || v.y || v.z || v.w }

all :: proc{
	all_bool,
	all_bvec2,
	all_bvec3,
	all_bvec4,
}
@(require_results) all_bool  :: proc "c" (v: bool) -> bool  { return v }
@(require_results) all_bvec2 :: proc "c" (v: bvec2) -> bool { return v.x && v.y }
@(require_results) all_bvec3 :: proc "c" (v: bvec3) -> bool { return v.x && v.y && v.z }
@(require_results) all_bvec4 :: proc "c" (v: bvec4) -> bool { return v.x && v.y && v.z && v.w }

not :: proc{
	not_bool,
	not_bvec2,
	not_bvec3,
	not_bvec4,
}
@(require_results) not_bool  :: proc "c" (v: bool) -> bool { return !v }
@(require_results) not_bvec2 :: proc "c" (v: bvec2) -> bvec2 { return {!v.x, !v.y} }
@(require_results) not_bvec3 :: proc "c" (v: bvec3) -> bvec3 { return {!v.x, !v.y, !v.z} }
@(require_results) not_bvec4 :: proc "c" (v: bvec4) -> bvec4 { return {!v.x, !v.y, !v.z, !v.w} }



/// Matrix Utilities

@(require_results) identity :: proc "c" ($M: typeid/matrix[$N, N]$T) -> M { return 1 }

@(require_results)
mat4Perspective :: proc "c" (fovy, aspect, near, far: f32) -> (m: mat4) {
	tan_half_fovy := tan(0.5 * fovy)
	m[0, 0] = 1 / (aspect*tan_half_fovy)
	m[1, 1] = 1 / (tan_half_fovy)
	m[2, 2] = -(far + near) / (far - near)
	m[3, 2] = -1
	m[2, 3] = -2*far*near / (far - near)
	return
}
@(require_results)
mat4PerspectiveInfinite :: proc "c" (fovy, aspect, near: f32) -> (m: mat4) {
	tan_half_fovy := tan(0.5 * fovy)
	m[0, 0] = 1 / (aspect*tan_half_fovy)
	m[1, 1] = 1 / (tan_half_fovy)
	m[2, 2] = -1
	m[3, 2] = -1
	m[2, 3] = -2*near
	return
}
@(require_results)
mat4Ortho3d :: proc "c" (left, right, bottom, top, near, far: f32) -> (m: mat4) {
	m[0, 0] = +2 / (right - left)
	m[1, 1] = +2 / (top - bottom)
	m[2, 2] = -2 / (far - near)
	m[0, 3] = -(right + left)   / (right - left)
	m[1, 3] = -(top   + bottom) / (top - bottom)
	m[2, 3] = -(far + near) / (far- near)
	m[3, 3] = 1
	return m
}
@(require_results)
mat4LookAt :: proc "c" (eye, centre, up: vec3) -> (m: mat4) {
	f := normalize(centre - eye)
	s := normalize(cross(f, up))
	u := cross(s, f)

	fe := dot(f, eye)
	
	m[0] = {+s.x, +u.x, -f.x, 0}
	m[1] = {+s.y, +u.y, -f.y, 0}
	m[2] = {+s.z, +u.z, -f.z, 0}
	m[3] = {-dot(s, eye), -dot(u, eye), +fe, 1}
	return
}
@(require_results)
mat4Rotate :: proc "c" (v: vec3, radians: f32) -> (rot: mat4) {
	c := cos(radians)
	s := sin(radians)

	a := normalize(v)
	t := a * (1-c)

	rot = 1

	rot[0, 0] = c + t[0]*a[0]
	rot[1, 0] = 0 + t[0]*a[1] + s*a[2]
	rot[2, 0] = 0 + t[0]*a[2] - s*a[1]
	rot[3, 0] = 0

	rot[0, 1] = 0 + t[1]*a[0] - s*a[2]
	rot[1, 1] = c + t[1]*a[1]
	rot[2, 1] = 0 + t[1]*a[2] + s*a[0]
	rot[3, 1] = 0

	rot[0, 2] = 0 + t[2]*a[0] + s*a[1]
	rot[1, 2] = 0 + t[2]*a[1] - s*a[0]
	rot[2, 2] = c + t[2]*a[2]
	rot[3, 2] = 0

	return rot
}
@(require_results)
mat4Translate :: proc "c" (v: vec3) -> (m: mat4) {
	m = 1
	m[3].xyz = v.xyz
	return
}
@(require_results)
mat4Scale :: proc "c" (v: vec3) -> (m: mat4) {
	m[0, 0] = v[0]
	m[1, 1] = v[1]
	m[2, 2] = v[2]
	m[3, 3] = 1
	return
}
@(require_results)
mat4Orientation :: proc "c" (normal, up: vec3) -> mat4 {
	if normal == up {
		return 1
	}
	
	rotation_axis := cross(up, normal)
	angle := acos(dot(normal, up))
	
	return mat4Rotate(rotation_axis, angle)
}
@(require_results)
mat4FromQuat :: proc "c" (q: quat) -> (m: mat4) {
	qxx := q.x * q.x
	qyy := q.y * q.y
	qzz := q.z * q.z
	qxz := q.x * q.z
	qxy := q.x * q.y
	qyz := q.y * q.z
	qwx := q.w * q.x
	qwy := q.w * q.y
	qwz := q.w * q.z

	m[0, 0] = 1 - 2 * (qyy + qzz)
	m[1, 0] = 2 * (qxy + qwz)
	m[2, 0] = 2 * (qxz - qwy)

	m[0, 1] = 2 * (qxy - qwz)
	m[1, 1] = 1 - 2 * (qxx + qzz)
	m[2, 1] = 2 * (qyz + qwx)

	m[0, 2] = 2 * (qxz + qwy)
	m[1, 2] = 2 * (qyz - qwx)
	m[2, 2] = 1 - 2 * (qxx + qyy)

	m[3, 3] = 1

	return
}


@(require_results)
dmat4Perspective :: proc "c" (fovy, aspect, near, far: f64) -> (m: dmat4) {
	tan_half_fovy := tan(0.5 * fovy)
	m[0, 0] = 1 / (aspect*tan_half_fovy)
	m[1, 1] = 1 / (tan_half_fovy)
	m[2, 2] = -(far + near) / (far - near)
	m[3, 2] = -1
	m[2, 3] = -2*far*near / (far - near)
	return
}
@(require_results)
dmat4PerspectiveInfinite :: proc "c" (fovy, aspect, near: f64) -> (m: dmat4) {
	tan_half_fovy := tan(0.5 * fovy)
	m[0, 0] = 1 / (aspect*tan_half_fovy)
	m[1, 1] = 1 / (tan_half_fovy)
	m[2, 2] = -1
	m[3, 2] = -1
	m[2, 3] = -2*near
	return
}
@(require_results)
dmat4Ortho3d :: proc "c" (left, right, bottom, top, near, far: f64) -> (m: dmat4) {
	m[0, 0] = +2 / (right - left)
	m[1, 1] = +2 / (top - bottom)
	m[2, 2] = -2 / (far - near)
	m[0, 3] = -(right + left)   / (right - left)
	m[1, 3] = -(top   + bottom) / (top - bottom)
	m[2, 3] = -(far + near) / (far- near)
	m[3, 3] = 1
	return m
}
@(require_results)
dmat4LookAt :: proc "c" (eye, centre, up: dvec3) -> (m: dmat4) {
	f := normalize(centre - eye)
	s := normalize(cross(f, up))
	u := cross(s, f)

	fe := dot(f, eye)
	
	m[0] = {+s.x, +u.x, -f.x, 0}
	m[1] = {+s.y, +u.y, -f.y, 0}
	m[2] = {+s.z, +u.z, -f.z, 0}
	m[3] = {-dot(s, eye), -dot(u, eye), +fe, 1}
	return
}
@(require_results)
dmat4Rotate :: proc "c" (v: dvec3, radians: f64) -> (rot: dmat4) {
	c := cos(radians)
	s := sin(radians)

	a := normalize(v)
	t := a * (1-c)

	rot = 1

	rot[0, 0] = c + t[0]*a[0]
	rot[1, 0] = 0 + t[0]*a[1] + s*a[2]
	rot[2, 0] = 0 + t[0]*a[2] - s*a[1]
	rot[3, 0] = 0

	rot[0, 1] = 0 + t[1]*a[0] - s*a[2]
	rot[1, 1] = c + t[1]*a[1]
	rot[2, 1] = 0 + t[1]*a[2] + s*a[0]
	rot[3, 1] = 0

	rot[0, 2] = 0 + t[2]*a[0] + s*a[1]
	rot[1, 2] = 0 + t[2]*a[1] - s*a[0]
	rot[2, 2] = c + t[2]*a[2]
	rot[3, 2] = 0

	return rot
}
@(require_results)
dmat4Translate :: proc "c" (v: dvec3) -> (m: dmat4) {
	m = 1
	m[3].xyz = v.xyz
	return
}
@(require_results)
dmat4Scale :: proc "c" (v: dvec3) -> (m: dmat4) {
	m[0, 0] = v[0]
	m[1, 1] = v[1]
	m[2, 2] = v[2]
	m[3, 3] = 1
	return
}
@(require_results)
dmat4Orientation :: proc "c" (normal, up: dvec3) -> dmat4 {
	if normal == up {
		return 1
	}
	
	rotation_axis := cross(up, normal)
	angle := acos(dot(normal, up))
	
	return dmat4Rotate(rotation_axis, angle)
}
@(require_results)
dmat4FromDquat :: proc "c" (q: dquat) -> (m: dmat4) {
	qxx := q.x * q.x
	qyy := q.y * q.y
	qzz := q.z * q.z
	qxz := q.x * q.z
	qxy := q.x * q.y
	qyz := q.y * q.z
	qwx := q.w * q.x
	qwy := q.w * q.y
	qwz := q.w * q.z

	m[0, 0] = 1 - 2 * (qyy + qzz)
	m[1, 0] = 2 * (qxy + qwz)
	m[2, 0] = 2 * (qxz - qwy)

	m[0, 1] = 2 * (qxy - qwz)
	m[1, 1] = 1 - 2 * (qxx + qzz)
	m[2, 1] = 2 * (qyz + qwx)

	m[0, 2] = 2 * (qxz + qwy)
	m[1, 2] = 2 * (qyz - qwx)
	m[2, 2] = 1 - 2 * (qxx + qyy)

	m[3, 3] = 1

	return
}

nlerp :: proc{
	quatNlerp,
	dquatNlerp,
}
slerp :: proc{
	quatSlerp,
	dquatSlerp,
}


@(require_results)
quatAxisAngle :: proc "c" (axis: vec3, radians: f32) -> (q: quat) {
	t := radians*0.5
	v := normalize(axis) * sin(t)
	q.x = v.x
	q.y = v.y
	q.z = v.z
	q.w = cos(t)
	return
}
@(require_results)
quatNlerp :: proc "c" (a, b: quat, t: f32) -> (c: quat) {
	c.x = a.x + (b.x-a.x)*t
	c.y = a.y + (b.y-a.y)*t
	c.z = a.z + (b.z-a.z)*t
	c.w = a.w + (b.w-a.w)*t
	return c/quat(builtin.abs(c))
}

@(require_results)
quatSlerp :: proc "c" (x, y: quat, t: f32) -> (q: quat) {
	a, b := x, y
	cos_angle := dot(a, b)
	if cos_angle < 0 {
		b = -b
		cos_angle = -cos_angle
	}
	if cos_angle > 1 - F32_EPSILON {
		q.x = a.x + (b.x-a.x)*t
		q.y = a.y + (b.y-a.y)*t
		q.z = a.z + (b.z-a.z)*t
		q.w = a.w + (b.w-a.w)*t
		return
	}

	angle := acos(cos_angle)
	sin_angle := sin(angle)
	factor_a := sin((1-t) * angle) / sin_angle
	factor_b := sin(t * angle)     / sin_angle

	q.x = factor_a * a.x + factor_b * b.x
	q.y = factor_a * a.y + factor_b * b.y
	q.z = factor_a * a.z + factor_b * b.z
	q.w = factor_a * a.w + factor_b * b.w
	return
}
@(require_results)
quatFromMat3 :: proc "c" (m: mat3) -> (q: quat) {
	four_x_squared_minus_1 := m[0, 0] - m[1, 1] - m[2, 2]
	four_y_squared_minus_1 := m[1, 1] - m[0, 0] - m[2, 2]
	four_z_squared_minus_1 := m[2, 2] - m[0, 0] - m[1, 1]
	four_w_squared_minus_1 := m[0, 0] + m[1, 1] + m[2, 2]

	biggest_index := 0
	four_biggest_squared_minus_1 := four_w_squared_minus_1
	if four_x_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_x_squared_minus_1
		biggest_index = 1
	}
	if four_y_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_y_squared_minus_1
		biggest_index = 2
	}
	if four_z_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_z_squared_minus_1
		biggest_index = 3
	}

	biggest_val := sqrt(four_biggest_squared_minus_1 + 1) * 0.5
	mult := 0.25 / biggest_val

	q = 1
	switch biggest_index {
	case 0:
		q.w = biggest_val
		q.x = (m[2, 1] - m[1, 2]) * mult
		q.y = (m[0, 2] - m[2, 0]) * mult
		q.z = (m[1, 0] - m[0, 1]) * mult
	case 1:
		q.w = (m[2, 1] - m[1, 2]) * mult
		q.x = biggest_val
		q.y = (m[1, 0] + m[0, 1]) * mult
		q.z = (m[0, 2] + m[2, 0]) * mult
	case 2:
		q.w = (m[0, 2] - m[2, 0]) * mult
		q.x = (m[1, 0] + m[0, 1]) * mult
		q.y = biggest_val
		q.z = (m[2, 1] + m[1, 2]) * mult
	case 3:
		q.w = (m[1, 0] - m[0, 1]) * mult
		q.x = (m[0, 2] + m[2, 0]) * mult
		q.y = (m[2, 1] + m[1, 2]) * mult
		q.z = biggest_val
	}
	return
}
@(require_results)
quatFromMat4 :: proc "c" (m: mat4) -> (q: quat) {
	return quatFromMat3(mat3(m))
}

@(require_results)
quatMulVec3 :: proc "c" (q: quat, v: vec3) -> vec3 {
	xyz := vec3{q.x, q.y, q.z}
	t := cross(2.0 * xyz, v)
	return v + q.w*t + cross(xyz, t)
}

@(require_results)
dquatAxisAngle :: proc "c" (axis: dvec3, radians: f64) -> (q: dquat) {
	t := radians*0.5
	v := normalize(axis) * sin(t)
	q.x = v.x
	q.y = v.y
	q.z = v.z
	q.w = cos(t)
	return
}
@(require_results)
dquatNlerp :: proc "c" (a, b: dquat, t: f64) -> (c: dquat) {
	c.x = a.x + (b.x-a.x)*t
	c.y = a.y + (b.y-a.y)*t
	c.z = a.z + (b.z-a.z)*t
	c.w = a.w + (b.w-a.w)*t
	return c/dquat(builtin.abs(c))
}

@(require_results)
dquatSlerp :: proc "c" (x, y: dquat, t: f64) -> (q: dquat) {
	a, b := x, y
	cos_angle := dot(a, b)
	if cos_angle < 0 {
		b = -b
		cos_angle = -cos_angle
	}
	if cos_angle > 1 - F64_EPSILON {
		q.x = a.x + (b.x-a.x)*t
		q.y = a.y + (b.y-a.y)*t
		q.z = a.z + (b.z-a.z)*t
		q.w = a.w + (b.w-a.w)*t
		return
	}

	angle := acos(cos_angle)
	sin_angle := sin(angle)
	factor_a := sin((1-t) * angle) / sin_angle
	factor_b := sin(t * angle)     / sin_angle

	q.x = factor_a * a.x + factor_b * b.x
	q.y = factor_a * a.y + factor_b * b.y
	q.z = factor_a * a.z + factor_b * b.z
	q.w = factor_a * a.w + factor_b * b.w
	return
}
@(require_results)
dquatFromdMat3 :: proc "c" (m: dmat3) -> (q: dquat) {
	four_x_squared_minus_1 := m[0, 0] - m[1, 1] - m[2, 2]
	four_y_squared_minus_1 := m[1, 1] - m[0, 0] - m[2, 2]
	four_z_squared_minus_1 := m[2, 2] - m[0, 0] - m[1, 1]
	four_w_squared_minus_1 := m[0, 0] + m[1, 1] + m[2, 2]

	biggest_index := 0
	four_biggest_squared_minus_1 := four_w_squared_minus_1
	if four_x_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_x_squared_minus_1
		biggest_index = 1
	}
	if four_y_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_y_squared_minus_1
		biggest_index = 2
	}
	if four_z_squared_minus_1 > four_biggest_squared_minus_1 {
		four_biggest_squared_minus_1 = four_z_squared_minus_1
		biggest_index = 3
	}

	biggest_val := sqrt(four_biggest_squared_minus_1 + 1) * 0.5
	mult := 0.25 / biggest_val

	q = 1
	switch biggest_index {
	case 0:
		q.w = biggest_val
		q.x = (m[2, 1] - m[1, 2]) * mult
		q.y = (m[0, 2] - m[2, 0]) * mult
		q.z = (m[1, 0] - m[0, 1]) * mult
	case 1:
		q.w = (m[2, 1] - m[1, 2]) * mult
		q.x = biggest_val
		q.y = (m[1, 0] + m[0, 1]) * mult
		q.z = (m[0, 2] + m[2, 0]) * mult
	case 2:
		q.w = (m[0, 2] - m[2, 0]) * mult
		q.x = (m[1, 0] + m[0, 1]) * mult
		q.y = biggest_val
		q.z = (m[2, 1] + m[1, 2]) * mult
	case 3:
		q.w = (m[1, 0] - m[0, 1]) * mult
		q.x = (m[0, 2] + m[2, 0]) * mult
		q.y = (m[2, 1] + m[1, 2]) * mult
		q.z = biggest_val
	}
	return
}
@(require_results)
dquatFromDmat4 :: proc "c" (m: dmat4) -> (q: dquat) {
	return dquatFromdMat3(dmat3(m))
}

@(require_results)
dquatMulDvec3 :: proc "c" (q: dquat, v: dvec3) -> dvec3 {
	xyz := dvec3{q.x, q.y, q.z}
	t := cross(2.0 * xyz, v)
	return v + q.w*t + cross(xyz, t)
}




@(require_results) inverse_mat2  :: proc "c" (m: mat2)  -> mat2  { return inverse_matrix2x2(m) }
@(require_results) inverse_mat3  :: proc "c" (m: mat3)  -> mat3  { return inverse_matrix3x3(m) }
@(require_results) inverse_mat4  :: proc "c" (m: mat4)  -> mat4  { return inverse_matrix4x4(m) }
@(require_results) inverse_dmat2 :: proc "c" (m: dmat2) -> dmat2 { return inverse_matrix2x2(m) }
@(require_results) inverse_dmat3 :: proc "c" (m: dmat3) -> dmat3 { return inverse_matrix3x3(m) }
@(require_results) inverse_dmat4 :: proc "c" (m: dmat4) -> dmat4 { return inverse_matrix4x4(m) }
@(require_results) inverse_quat  :: proc "c" (q: quat)  -> quat  { return 1/q }
@(require_results) inverse_dquat :: proc "c" (q: dquat) -> dquat { return 1/q }


transpose :: intrinsics.transpose


determinant :: proc{
	determinant_matrix1x1,
	determinant_matrix2x2,
	determinant_matrix3x3,
	determinant_matrix4x4,
}

adjugate :: proc{
	adjugate_matrix1x1,
	adjugate_matrix2x2,
	adjugate_matrix3x3,
	adjugate_matrix4x4,
}

cofactor :: proc{
	cofactor_matrix1x1,
	cofactor_matrix2x2,
	cofactor_matrix3x3,
	cofactor_matrix4x4,
}

inverse_transpose :: proc{
	inverse_transpose_matrix1x1,
	inverse_transpose_matrix2x2,
	inverse_transpose_matrix3x3,
	inverse_transpose_matrix4x4,
}


inverse :: proc{
	inverse_matrix1x1,
	inverse_matrix2x2,
	inverse_matrix3x3,
	inverse_matrix4x4,
}

@(require_results)
hermitian_adjoint :: proc "contextless" (m: $M/matrix[$N, N]$T) -> M where intrinsics.type_is_complex(T), N >= 1 {
	return conj(transpose(m))
}

@(require_results)
trace :: proc "contextless" (m: $M/matrix[$N, N]$T) -> (trace: T) {
	for i in 0..<N {
		trace += m[i, i]
	}
	return
}

@(require_results)
matrix_minor :: proc "contextless" (m: $M/matrix[$N, N]$T, #any_int row, column: int) -> (minor: T) where N > 1 {
	K :: int(N-1)
	cut_down: matrix[K, K]T
	for col_idx in 0..<K {
		j := col_idx + int(col_idx >= column)
		for row_idx in 0..<K {
			i := row_idx + int(row_idx >= row)
			cut_down[row_idx, col_idx] = m[i, j]
		}
	}
	return determinant(cut_down)
}



@(require_results)
determinant_matrix1x1 :: proc "contextless" (m: $M/matrix[1, 1]$T) -> (det: T) {
	return m[0, 0]
}

@(require_results)
determinant_matrix2x2 :: proc "contextless" (m: $M/matrix[2, 2]$T) -> (det: T) {
	return m[0, 0]*m[1, 1] - m[0, 1]*m[1, 0]
}
@(require_results)
determinant_matrix3x3 :: proc "contextless" (m: $M/matrix[3, 3]$T) -> (det: T) {
	a := +m[0, 0] * (m[1, 1] * m[2, 2] - m[1, 2] * m[2, 1])
	b := -m[0, 1] * (m[1, 0] * m[2, 2] - m[1, 2] * m[2, 0])
	c := +m[0, 2] * (m[1, 0] * m[2, 1] - m[1, 1] * m[2, 0])
	return a + b + c
}
@(require_results)
determinant_matrix4x4 :: proc "contextless" (m: $M/matrix[4, 4]$T) -> (det: T) {
	c := cofactor(m)
	#no_bounds_check for i in 0..<4 {
		det += m[0, i] * c[0, i]
	}
	return
}




@(require_results)
adjugate_matrix1x1 :: proc "contextless" (x: $M/matrix[1, 1]$T) -> (y: M) {
	y = x
	return
}

@(require_results)
adjugate_matrix2x2 :: proc "contextless" (x: $M/matrix[2, 2]$T) -> (y: M) {
	y[0, 0] = +x[1, 1]
	y[0, 1] = -x[0, 1]
	y[1, 0] = -x[1, 0]
	y[1, 1] = +x[0, 0]
	return
}

@(require_results)
adjugate_matrix3x3 :: proc "contextless" (m: $M/matrix[3, 3]$T) -> (y: M) {
	y[0, 0] = +(m[1, 1] * m[2, 2] - m[2, 1] * m[1, 2])
	y[1, 0] = -(m[1, 0] * m[2, 2] - m[2, 0] * m[1, 2])
	y[2, 0] = +(m[1, 0] * m[2, 1] - m[2, 0] * m[1, 1])
	y[0, 1] = -(m[0, 1] * m[2, 2] - m[2, 1] * m[0, 2])
	y[1, 1] = +(m[0, 0] * m[2, 2] - m[2, 0] * m[0, 2])
	y[2, 1] = -(m[0, 0] * m[2, 1] - m[2, 0] * m[0, 1])
	y[0, 2] = +(m[0, 1] * m[1, 2] - m[1, 1] * m[0, 2])
	y[1, 2] = -(m[0, 0] * m[1, 2] - m[1, 0] * m[0, 2])
	y[2, 2] = +(m[0, 0] * m[1, 1] - m[1, 0] * m[0, 1])
	return
}

@(require_results)
adjugate_matrix4x4 :: proc "contextless" (x: $M/matrix[4, 4]$T) -> (y: M) {
	for i in 0..<4 {
		for j in 0..<4 {
			sign: T = 1 if (i + j) % 2 == 0 else -1
			y[i, j] = sign * matrix_minor(x, j, i)
		}
	}
	return
}


@(require_results)
cofactor_matrix1x1 :: proc "contextless" (x: $M/matrix[1, 1]$T) -> (y: M) {
	y = x
	return
}

@(require_results)
cofactor_matrix2x2 :: proc "contextless" (x: $M/matrix[2, 2]$T) -> (y: M) {
	y[0, 0] = +x[1, 1]
	y[0, 1] = -x[1, 0]
	y[1, 0] = -x[0, 1]
	y[1, 1] = +x[0, 0]
	return
}

@(require_results)
cofactor_matrix3x3 :: proc "contextless" (m: $M/matrix[3, 3]$T) -> (y: M) {
	y[0, 0] = +(m[1, 1] * m[2, 2] - m[2, 1] * m[1, 2])
	y[0, 1] = -(m[1, 0] * m[2, 2] - m[2, 0] * m[1, 2])
	y[0, 2] = +(m[1, 0] * m[2, 1] - m[2, 0] * m[1, 1])
	y[1, 0] = -(m[0, 1] * m[2, 2] - m[2, 1] * m[0, 2])
	y[1, 1] = +(m[0, 0] * m[2, 2] - m[2, 0] * m[0, 2])
	y[1, 2] = -(m[0, 0] * m[2, 1] - m[2, 0] * m[0, 1])
	y[2, 0] = +(m[0, 1] * m[1, 2] - m[1, 1] * m[0, 2])
	y[2, 1] = -(m[0, 0] * m[1, 2] - m[1, 0] * m[0, 2])
	y[2, 2] = +(m[0, 0] * m[1, 1] - m[1, 0] * m[0, 1])
	return
}


@(require_results)
cofactor_matrix4x4 :: proc "contextless" (x: $M/matrix[4, 4]$T) -> (y: M) {
	for i in 0..<4 {
		for j in 0..<4 {
			sign: T = 1 if (i + j) % 2 == 0 else -1
			y[i, j] = sign * matrix_minor(x, i, j)
		}
	}
	return
}

@(require_results)
inverse_transpose_matrix1x1 :: proc "contextless" (x: $M/matrix[1, 1]$T) -> (y: M) {
	y[0, 0] = 1/x[0, 0]
	return
}

@(require_results)
inverse_transpose_matrix2x2 :: proc "contextless" (x: $M/matrix[2, 2]$T) -> (y: M) {
	d := x[0, 0]*x[1, 1] - x[0, 1]*x[1, 0]
	when intrinsics.type_is_integer(T) {
		y[0, 0] = +x[1, 1] / d
		y[1, 0] = -x[0, 1] / d
		y[0, 1] = -x[1, 0] / d
		y[1, 1] = +x[0, 0] / d
	} else {
		id := 1 / d
		y[0, 0] = +x[1, 1] * id
		y[1, 0] = -x[0, 1] * id
		y[0, 1] = -x[1, 0] * id
		y[1, 1] = +x[0, 0] * id
	}
	return
}

@(require_results)
inverse_transpose_matrix3x3 :: proc "contextless" (x: $M/matrix[3, 3]$T) -> (y: M) #no_bounds_check {
	c := cofactor(x)
	d := determinant(x)
	when intrinsics.type_is_integer(T) {
		for i in 0..<3 {
			for j in 0..<3 {
				y[i, j] = c[i, j] / d
			}
		}
	} else {
		id := 1/d
		for i in 0..<3 {
			for j in 0..<3 {
				y[i, j] = c[i, j] * id
			}
		}
	}
	return
}

@(require_results)
inverse_transpose_matrix4x4 :: proc "contextless" (x: $M/matrix[4, 4]$T) -> (y: M) #no_bounds_check {
	c := cofactor(x)
	d: T
	for i in 0..<4 {
		d += x[0, i] * c[0, i]
	}
	when intrinsics.type_is_integer(T) {
		for i in 0..<4 {
			for j in 0..<4 {
				y[i, j] = c[i, j] / d
			}
		}
	} else {
		id := 1/d
		for i in 0..<4 {
			for j in 0..<4 {
				y[i, j] = c[i, j] * id
			}
		}
	}
	return
}

@(require_results)
inverse_matrix1x1 :: proc "contextless" (x: $M/matrix[1, 1]$T) -> (y: M) {
	y[0, 0] = 1/x[0, 0]
	return
}

@(require_results)
inverse_matrix2x2 :: proc "contextless" (x: $M/matrix[2, 2]$T) -> (y: M) {
	d := x[0, 0]*x[1, 1] - x[0, 1]*x[1, 0]
	when intrinsics.type_is_integer(T) {
		y[0, 0] = +x[1, 1] / d
		y[0, 1] = -x[0, 1] / d
		y[1, 0] = -x[1, 0] / d
		y[1, 1] = +x[0, 0] / d
	} else {
		id := 1 / d
		y[0, 0] = +x[1, 1] * id
		y[0, 1] = -x[0, 1] * id
		y[1, 0] = -x[1, 0] * id
		y[1, 1] = +x[0, 0] * id
	}
	return
}

@(require_results)
inverse_matrix3x3 :: proc "contextless" (x: $M/matrix[3, 3]$T) -> (y: M) #no_bounds_check {
	c := cofactor(x)
	d := determinant(x)
	when intrinsics.type_is_integer(T) {
		for i in 0..<3 {
			for j in 0..<3 {
				y[i, j] = c[j, i] / d
			}
		}
	} else {
		id := 1/d
		for i in 0..<3 {
			for j in 0..<3 {
				y[i, j] = c[j, i] * id
			}
		}
	}
	return
}

@(require_results)
inverse_matrix4x4 :: proc "contextless" (x: $M/matrix[4, 4]$T) -> (y: M) #no_bounds_check {
	c := cofactor(x)
	d: T
	for i in 0..<4 {
		d += x[0, i] * c[0, i]
	}
	when intrinsics.type_is_integer(T) {
		for i in 0..<4 {
			for j in 0..<4 {
				y[i, j] = c[j, i] / d
			}
		}
	} else {
		id := 1/d
		for i in 0..<4 {
			for j in 0..<4 {
				y[i, j] = c[j, i] * id
			}
		}
	}
	return
}
