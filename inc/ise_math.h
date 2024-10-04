// InfiniScroll Engine
// ise_math.h
// Math routines
//
// Kamal Pillai
// Based of ise_ library (04/04/2009)

#ifndef __ISE_MATH_H
#define __ISE_MATH_H

const float ISE_PI = 3.1415926535898f;

inline float rad2deg( float x )
{
  return (180*x) / ISE_PI;
}

inline float deg2rad( float x )
{
  return (ISE_PI*x) / 180;
}

// operations to a single vector
float* ise_v_negate( uint32_t vec_length, float* d );
float* ise_v_swizzle( uint32_t vec_length, float* d, uint32_t* indices );
float  ise_v_length( uint32_t vec_length, float* s );

// operations of two vectors of the same length
float* ise_v_add ( uint32_t vec_length, float* d, float* s1, float* s2 );
float* ise_v_sub ( uint32_t vec_length, float* d, float* s1, float* s2 );
float* ise_v_mul ( uint32_t vec_length, float* d, float* s1, float* s2 );
float* ise_v_div ( uint32_t vec_length, float* d, float* s1, float* s2 );
float* ise_v_cross( uint32_t vec_length, float* d, float* s1, float* s2 );
bool ise_v_equals  ( uint32_t vec_length, float* s1, float* s2 );
float ise_v_dot  ( uint32_t vec_length, float* s1, float* s2 );
float* ise_v_min ( uint32_t vec_length, float* d, float* s1, float* s2 );
float* ise_v_max ( uint32_t vec_length, float* d, float* s1, float* s2 );

// operations where s1 is scalar and s2 is a vector
float* ise_sv_add( uint32_t vec_length, float* d, float s1, float* s2 );
float* ise_sv_sub( uint32_t vec_length, float* d, float s1, float* s2 );
float* ise_sv_mul( uint32_t vec_length, float* d, float s1, float* s2 );
float* ise_sv_div( uint32_t vec_length, float* d, float s1, float* s2 );

// operations on a single matrix
float  ise_m_determinant  ( uint32_t rows, float* s );
float* ise_m_transpose    ( uint32_t rows, uint32_t cols, float* d );
float* ise_m_inverse      ( uint32_t rows, float* d );
float* ise_m_swizzle      ( uint32_t rows, uint32_t cols, float* d, uint32_t* row_indices, uint32_t* col_indices );
float* ise_m_set_identity ( uint32_t rows, float* d );
float* ise_m_set_rotation ( uint32_t rows, float* d, float angle, float* axis );

// operations on a single 4x4 matrix
float* ise_m4_set_perspective_off_center( float* d, float left, float right, float bottom, float top, float znear, float zfar, bool left_handed, bool dx_style );
float* ise_m4_set_perspective_fov( float* d, float fovy, float aspect, float znear, float zfar, bool left_handed, bool dx_style  );
float* ise_m4_set_ortho_offcenter( float* d, float left, float right, float bottom, float top, float znear, float zfar, bool left_handed, bool dx_style  );
float* ise_m4_set_look_at( float* d, float* eye, float* at, float* up_dir, bool left_handed );

// operations on 2 matrices
float* ise_m_mul( uint32_t s1_rows, uint32_t s2_rows, uint32_t s2_cols, float* d, float* s1, float* s2 );

// shortcuts to mornalize vector
#define ise_v_normalize(l,d) ise_sv_mul( (l), (d), 1.0f / ise_v_length( (l), (d) ), (d) )

// shortcuts for vector to scalar operations
#define ise_vs_add(l,d,s1,s2) ise_sv_add( (l), (d),  (s2), (s1) )
#define ise_vs_sub(l,d,s1,s2) ise_sv_add( (l), (d), -(s2), (s1) )
#define ise_vs_mul(l,d,s1,s2) ise_sv_mul( (l), (d),  (s2), (s1) )
#define ise_vs_div(l,d,s1,s2) ise_sv_mul( (l), (d), 1.0f/(s2), (s1) )

// shortcut for not equals
#define ise_v_not_equals(l,s1,s2) (!ise_v_equals(l,s1,s2))

// shortcuts for vectors of length 2 through 4  operations
// also shortcuts for 2x2, 3x3, and 4x4 matrices

#define ise_v2_negate(d) ise_v_negate(  2, (d) )
#define ise_v3_negate(d) ise_v_negate(  3, (d) )
#define ise_v4_negate(d) ise_v_negate(  4, (d) )
#define ise_m2_negate(d) ise_v_negate(  4, (d) )
#define ise_m3_negate(d) ise_v_negate(  9, (d) )
#define ise_m4_negate(d) ise_v_negate( 16, (d) )

#define ise_v2_normalize(d) ise_v_normalize( 2, (d) )
#define ise_v3_normalize(d) ise_v_normalize( 3, (d) )
#define ise_v4_normalize(d) ise_v_normalize( 4, (d) )

#define ise_v2_swizzle(d, i) ise_v_swizzle( 2, (d), (i) )
#define ise_v3_swizzle(d, i) ise_v_swizzle( 3, (d), (i) )
#define ise_v4_swizzle(d, i) ise_v_swizzle( 4, (d), (i) )
#define ise_m2_swizzle(d, r, c) ise_m_swizzle( 2, 2, (d), (r), (c) )
#define ise_m3_swizzle(d, r, c) ise_m_swizzle( 3, 3, (d), (r), (c) )
#define ise_m4_swizzle(d, r, c) ise_m_swizzle( 4, 4, (d), (r), (c) )

#define ise_v2_length(s) ise_v_length( 2, (s) )
#define ise_v3_length(s) ise_v_length( 3, (s) )
#define ise_v4_length(s) ise_v_length( 4, (s) )

#define ise_v2_add(d,s1,s2) ise_v_add(  2, (d), (s1), (s2) )
#define ise_v3_add(d,s1,s2) ise_v_add(  3, (d), (s1), (s2) )
#define ise_v4_add(d,s1,s2) ise_v_add(  4, (d), (s1), (s2) )
#define ise_m2_add(d,s1,s2) ise_v_add(  4, (d), (s1), (s2) )
#define ise_m3_add(d,s1,s2) ise_v_add(  9, (d), (s1), (s2) )
#define ise_m4_add(d,s1,s2) ise_v_add( 16, (d), (s1), (s2) )
#define ise_v2_sub(d,s1,s2) ise_v_sub(  2, (d), (s1), (s2) )
#define ise_v3_sub(d,s1,s2) ise_v_sub(  3, (d), (s1), (s2) )
#define ise_v4_sub(d,s1,s2) ise_v_sub(  4, (d), (s1), (s2) )
#define ise_m2_sub(d,s1,s2) ise_v_sub(  4, (d), (s1), (s2) )
#define ise_m3_sub(d,s1,s2) ise_v_sub(  9, (d), (s1), (s2) )
#define ise_m4_sub(d,s1,s2) ise_v_sub( 16, (d), (s1), (s2) )
#define ise_v2_mul(d,s1,s2) ise_v_mul(  2, (d), (s1), (s2) )
#define ise_v3_mul(d,s1,s2) ise_v_mul(  3, (d), (s1), (s2) )
#define ise_v4_mul(d,s1,s2) ise_v_mul(  4, (d), (s1), (s2) )
#define ise_m2_component_mul(d,s1,s2) ise_v_mul(  4, (d), (s1), (s2) )
#define ise_m3_component_mul(d,s1,s2) ise_v_mul(  9, (d), (s1), (s2) )
#define ise_m4_component_mul(d,s1,s2) ise_v_mul( 16, (d), (s1), (s2) )
#define ise_v2_div(d,s1,s2) ise_v_div(  2, (d), (s1), (s2) )
#define ise_v3_div(d,s1,s2) ise_v_div(  3, (d), (s1), (s2) )
#define ise_v4_div(d,s1,s2) ise_v_div(  4, (d), (s1), (s2) )
#define ise_m2_div(d,s1,s2) ise_v_div(  4, (d), (s1), (s2) )
#define ise_m3_div(d,s1,s2) ise_v_div(  9, (d), (s1), (s2) )
#define ise_m4_div(d,s1,s2) ise_v_div( 16, (d), (s1), (s2) )

#define ise_sv2_add(d,s1,s2) ise_sv_add(  2, (d), (s1), (s2) )
#define ise_sv3_add(d,s1,s2) ise_sv_add(  3, (d), (s1), (s2) )
#define ise_sv4_add(d,s1,s2) ise_sv_add(  4, (d), (s1), (s2) )
#define ise_sm2_add(d,s1,s2) ise_sv_add(  4, (d), (s1), (s2) )
#define ise_sm3_add(d,s1,s2) ise_sv_add(  9, (d), (s1), (s2) )
#define ise_sm4_add(d,s1,s2) ise_sv_add( 16, (d), (s1), (s2) )
#define ise_sv2_sub(d,s1,s2) ise_sv_sub(  2, (d), (s1), (s2) )
#define ise_sv3_sub(d,s1,s2) ise_sv_sub(  3, (d), (s1), (s2) )
#define ise_sv4_sub(d,s1,s2) ise_sv_sub(  4, (d), (s1), (s2) )
#define ise_sm2_sub(d,s1,s2) ise_sv_sub(  4, (d), (s1), (s2) )
#define ise_sm3_sub(d,s1,s2) ise_sv_sub(  9, (d), (s1), (s2) )
#define ise_sm4_sub(d,s1,s2) ise_sv_sub( 16, (d), (s1), (s2) )
#define ise_sv2_mul(d,s1,s2) ise_sv_mul(  2, (d), (s1), (s2) )
#define ise_sv3_mul(d,s1,s2) ise_sv_mul(  3, (d), (s1), (s2) )
#define ise_sv4_mul(d,s1,s2) ise_sv_mul(  4, (d), (s1), (s2) )
#define ise_sm2_mul(d,s1,s2) ise_sv_mul(  4, (d), (s1), (s2) )
#define ise_sm3_mul(d,s1,s2) ise_sv_mul(  9, (d), (s1), (s2) )
#define ise_sm4_mul(d,s1,s2) ise_sv_mul( 16, (d), (s1), (s2) )
#define ise_sv2_div(d,s1,s2) ise_sv_div(  2, (d), (s1), (s2) )
#define ise_sv3_div(d,s1,s2) ise_sv_div(  3, (d), (s1), (s2) )
#define ise_sv4_div(d,s1,s2) ise_sv_div(  4, (d), (s1), (s2) )
#define ise_sm2_div(d,s1,s2) ise_sv_div(  4, (d), (s1), (s2) )
#define ise_sm3_div(d,s1,s2) ise_sv_div(  9, (d), (s1), (s2) )
#define ise_sm4_div(d,s1,s2) ise_sv_div( 16, (d), (s1), (s2) )

#define ise_v2s_add(d,s1,s2) ise_vs_add(  2, (d), (s1), (s2) )
#define ise_v3s_add(d,s1,s2) ise_vs_add(  3, (d), (s1), (s2) )
#define ise_v4s_add(d,s1,s2) ise_vs_add(  4, (d), (s1), (s2) )
#define ise_m2s_add(d,s1,s2) ise_vs_add(  4, (d), (s1), (s2) )
#define ise_m3s_add(d,s1,s2) ise_vs_add(  9, (d), (s1), (s2) )
#define ise_m4s_add(d,s1,s2) ise_vs_add( 16, (d), (s1), (s2) )
#define ise_v2s_sub(d,s1,s2) ise_vs_sub(  2, (d), (s1), (s2) )
#define ise_v3s_sub(d,s1,s2) ise_vs_sub(  3, (d), (s1), (s2) )
#define ise_v4s_sub(d,s1,s2) ise_vs_sub(  4, (d), (s1), (s2) )
#define ise_m2s_sub(d,s1,s2) ise_vs_sub(  4, (d), (s1), (s2) )
#define ise_m3s_sub(d,s1,s2) ise_vs_sub(  9, (d), (s1), (s2) )
#define ise_m4s_sub(d,s1,s2) ise_vs_sub( 16, (d), (s1), (s2) )
#define ise_v2s_mul(d,s1,s2) ise_vs_mul(  2, (d), (s1), (s2) )
#define ise_v3s_mul(d,s1,s2) ise_vs_mul(  3, (d), (s1), (s2) )
#define ise_v4s_mul(d,s1,s2) ise_vs_mul(  4, (d), (s1), (s2) )
#define ise_m2s_mul(d,s1,s2) ise_vs_mul(  4, (d), (s1), (s2) )
#define ise_m3s_mul(d,s1,s2) ise_vs_mul(  9, (d), (s1), (s2) )
#define ise_m4s_mul(d,s1,s2) ise_vs_mul( 16, (d), (s1), (s2) )
#define ise_v2s_div(d,s1,s2) ise_vs_div(  2, (d), (s1), (s2) )
#define ise_v3s_div(d,s1,s2) ise_vs_div(  3, (d), (s1), (s2) )
#define ise_v4s_div(d,s1,s2) ise_vs_div(  4, (d), (s1), (s2) )
#define ise_m2s_div(d,s1,s2) ise_vs_div(  4, (d), (s1), (s2) )
#define ise_m3s_div(d,s1,s2) ise_vs_div(  9, (d), (s1), (s2) )
#define ise_m4s_div(d,s1,s2) ise_vs_div( 16, (d), (s1), (s2) )

#define ise_v2_cross(d,s1,s2) ise_v_cross( 2, (d), (s1), (s2) )
#define ise_v3_cross(d,s1,s2) ise_v_cross( 3, (d), (s1), (s2) )

#define ise_v2_equals(s1,s2) ise_v_equals(  2, (s1), (s2) )
#define ise_v3_equals(s1,s2) ise_v_equals(  3, (s1), (s2) )
#define ise_v4_equals(s1,s2) ise_v_equals(  4, (s1), (s2) )
#define ise_m2_equals(s1,s2) ise_v_equals(  4, (s1), (s2) )
#define ise_m3_equals(s1,s2) ise_v_equals(  9, (s1), (s2) )
#define ise_m4_equals(s1,s2) ise_v_equals( 16, (s1), (s2) )
#define ise_v2_not_equals(s1,s2) ise_v_notequals(  2, (s1), (s2) )
#define ise_v3_not_equals(s1,s2) ise_v_notequals(  3, (s1), (s2) )
#define ise_v4_not_equals(s1,s2) ise_v_notequals(  4, (s1), (s2) )
#define ise_m2_not_equals(s1,s2) ise_v_notequals(  4, (s1), (s2) )
#define ise_m3_not_equals(s1,s2) ise_v_notequals(  9, (s1), (s2) )
#define ise_m4_not_equals(s1,s2) ise_v_notequals( 16, (s1), (s2) )

#define ise_v2_dot(s1,s2) ise_v_dot( 2, (s1), (s2) )
#define ise_v3_dot(s1,s2) ise_v_dot( 3, (s1), (s2) )
#define ise_v4_dot(s1,s2) ise_v_dot( 4, (s1), (s2) )

#define ise_v2_min(d,s1,s2) ise_v_min( 2, (d), (s1), (s2) )
#define ise_v3_min(d,s1,s2) ise_v_min( 3, (d), (s1), (s2) )
#define ise_v4_min(d,s1,s2) ise_v_min( 4, (d), (s1), (s2) )
#define ise_v2_max(d,s1,s2) ise_v_max( 2, (d), (s1), (s2) )
#define ise_v3_max(d,s1,s2) ise_v_max( 3, (d), (s1), (s2) )
#define ise_v4_max(d,s1,s2) ise_v_max( 4, (d), (s1), (s2) )

#define ise_m2_determinant(s1) ise_m_determinant( 2, (s1) )
#define ise_m3_determinant(s1) ise_m_determinant( 3, (s1) )
#define ise_m4_determinant(s1) ise_m_determinant( 4, (s1) )
#define ise_m2_transpose(d) ise_m_transpose( 2, 2, (d) )
#define ise_m3_transpose(d) ise_m_transpose( 3, 3, (d) )
#define ise_m4_transpose(d) ise_m_transpose( 4, 4, (d) )
#define ise_m2_inverse(d) ise_m_inverse( 2, (d) )
#define ise_m3_inverse(d) ise_m_inverse( 3, (d) )
#define ise_m4_inverse(d) ise_m_inverse( 4, (d) )
#define ise_m2_set_identity(d) ise_m_set_identity( 2, (d) )
#define ise_m3_set_identity(d) ise_m_set_identity( 3, (d) )
#define ise_m4_set_identity(d) ise_m_set_identity( 4, (d) )
#define ise_m2_set_rotation(d,a)   ise_m_set_rotation( 2, (d), (a), null )
#define ise_m3_set_rotation(d,a,x) ise_m_set_rotation( 3, (d), (a), (x) )
#define ise_m4_set_rotation(d,a,x) ise_m_set_rotation( 4, (d), (a), (x) )

#define ise_m4_set_perspective_off_centerlh(d,l,r,b,t,n,f,s) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), true, (s) )
#define ise_m4_set_perspective_off_centerrh(d,l,r,b,t,n,f,s) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), false, (s) )
#define ise_m4_set_perspective_lh(d,w,h,n,f,s) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, (s) )
#define ise_m4_set_perspective_rh(d,w,h,n,f,s) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, (s) )
#define ise_m4_set_perspective_fov_lh(d,fy,a,n,f,s) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), true, (s) )
#define ise_m4_set_perspective_fov_rh(d,fy,a,n,f,s) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), false, (s) )
#define ise_m4_set_ortho_off_center_lh(d,l,r,b,t,n,f,s) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), true, (s) )
#define ise_m4_set_ortho_off_center_rh(d,l,r,b,t,n,f,s) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), false, (s) )
#define ise_m4_set_ortho_lh(d,w,h,n,f,s) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, (s) )
#define ise_m4_set_ortho_rh(d,w,h,n,f,s) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, (s) )

#define ise_m4_set_dx_perspective_off_center_lh(d,l,r,b,t,n,f) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), true, true )
#define ise_m4_set_dx_perspective_off_center_rh(d,l,r,b,t,n,f) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), false, true )
#define ise_m4_set_dx_perspective_lh(d,w,h,n,f) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, true )
#define ise_m4_set_dx_perspective_rh(d,w,h,n,f) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, true )
#define ise_m4_set_dx_perspective_fov_lh(d,fy,a,n,f) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), true, true )
#define ise_m4_set_dx_perspective_fov_rh(d,fy,a,n,f) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), false, true )
#define ise_m4_set_dx_ortho_off_center_lh(d,l,r,b,t,n,f) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), true, true )
#define ise_m4_set_dx_ortho_off_center_rh(d,l,r,b,t,n,f) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), false, true )
#define ise_m4_set_dx_ortho_lh(d,w,h,n,f) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, true )
#define ise_m4_set_dx_ortho_rh(d,w,h,n,f) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, true )

#define ise_m4_set_gl_perspective_off_center_lh(d,l,r,b,t,n,f) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), true, false )
#define ise_m4_set_gl_perspective_off_center_rh(d,l,r,b,t,n,f) ise_m4_set_perspective_off_center( (d), (l), (r), (b), (t), (n), (f), false, false )
#define ise_m4_set_gl_perspective_lh(d,w,h,n,f) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, false )
#define ise_m4_set_gl_perspective_rh(d,w,h,n,f) ise_m4_set_perspective_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, false )
#define ise_m4_set_gl_perspective_fov_lh(d,fy,a,n,f) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), true, false )
#define ise_m4_set_gl_perspective_fov_rh(d,fy,a,n,f) ise_m4_set_perspective_fov( (d), (fy), (a), (n), (f), false, false )
#define ise_m4_set_gl_ortho_off_center_lh(d,l,r,b,t,n,f) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), true, false )
#define ise_m4_set_gl_ortho_off_center_rh(d,l,r,b,t,n,f) ise_m4_set_ortho_off_center( (d), (l), (r), (b), (t), (n), (f), false, false )
#define ise_m4_set_gl_ortho_lh(d,w,h,n,f) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), true, false )
#define ise_m4_set_gl_ortho_rh(d,w,h,n,f) ise_m4_set_ortho_off_center( (d), -(w)/2.0f, (w)/2.0f, -(h)/2.0f, (h)/2.0f, (n), (f), false, false )

#define ise_m4_set_look_at_lh(d,e,a,u) ise_m4_set_look_at( (d), (e), (a), (u), true )
#define ise_m4_set_look_at_rh(d,e,a,u) ise_m4_set_look_at( (d), (e), (a), (u), false )

#define ise_m2_mul(d,s1,s2) ise_m_mul( 2, 2, 2, (d), (s1), (s2) )
#define ise_m3_mul(d,s1,s2) ise_m_mul( 3, 3, 3, (d), (s1), (s2) )
#define ise_m4_mul(d,s1,s2) ise_m_mul( 4, 4, 4, (d), (s1), (s2) )

#define ise_vm2_mul(d,s1,s2) ise_m_mul( 1, 2, 2, (d), (s1), (s2) )
#define ise_vm3_mul(d,s1,s2) ise_m_mul( 1, 3, 3, (d), (s1), (s2) )
#define ise_vm4_mul(d,s1,s2) ise_m_mul( 1, 4, 4, (d), (s1), (s2) )

#define ise_mv2_mul(d,s1,s2) ise_m_mul( 2, 2, 1, (d), (s1), (s2) )
#define ise_mv3_mul(d,s1,s2) ise_m_mul( 3, 3, 1, (d), (s1), (s2) )
#define ise_mv4_mul(d,s1,s2) ise_m_mul( 4, 4, 1, (d), (s1), (s2) )


#endif  //  __ISE_MATH_H
