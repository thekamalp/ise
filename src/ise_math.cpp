// InfiniScroll Engine
// ise_math.cpp
// Math routines
//
// Kamal Pillai
// Based of k2 library (04/04/2009)

#include "ise.h"

#define ISE_M_STATIC_ARRAY_SIZE 16

//  -----------------------------------------------------------------------------
//  Scalar and vector opeations

float* ise_sv_add( uint32_t vec_length, float* d, float s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s2++ ) *dp = s1 + *s2;
  return d;
}

float* ise_sv_sub( uint32_t vec_length, float* d, float s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s2++ ) *dp = s1 - *s2;
  return d;
}

float* ise_sv_mul( uint32_t vec_length, float* d, float s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s2++ ) *dp = s1 * *s2;
  return d;
}

float* ise_sv_div( uint32_t vec_length, float* d, float s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s2++ ) *dp = s1 / *s2;
  return d;
}

//  ---------------------------------------------------------------------------
//  Single argument vector functions

float* ise_v_negate( uint32_t vec_length, float* d )
{
  uint32_t i;
  for( i=0; i<vec_length; i++ ) d[i] = -d[i];
  return d;
}

float* ise_v_swizzle( uint32_t vec_length, float* d, uint32_t* indices )
{
  uint32_t i;
  float static_copy[ISE_M_STATIC_ARRAY_SIZE];
  float* temp_copy;

  if( vec_length > ISE_M_STATIC_ARRAY_SIZE )
    temp_copy = new float[vec_length];
  else
    temp_copy = static_copy;

  for( i=0; i<vec_length; i++ )
    temp_copy[i] = d[ indices[i] ];

  if( temp_copy != static_copy ) {
    memcpy( d, temp_copy, vec_length*sizeof(float) );
    delete [] temp_copy;
  } else {
    for( i=0; i<vec_length; i++ )
      d[i] = temp_copy[i];
  }

  return d;
}

float ise_v_length( uint32_t vec_length, float* s )
{
  return sqrt( ise_v_dot( vec_length, s, s ) );
}

//  -----------------------------------------------------------------------------
//  Two argument vector functions

float* ise_v_add ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = *s1 + *s2;
  return d;
}

float* ise_v_sub ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = *s1 - *s2;
  return d;
}

float* ise_v_mul ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = *s1 * *s2;
  return d;
}

float* ise_v_div ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float *dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = *s1 / *s2;
  return d;
}

float* ise_v_cross( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  switch( vec_length ) {
  case 2:
    *d = s1[0]*s2[1] - s1[1]*s2[0];
    break;
  case 3:
    {
      float temp_copy[3];
      temp_copy[0] = s1[1]*s2[2] - s1[2]*s2[1];
      temp_copy[1] = s1[2]*s2[0] - s1[0]*s2[2];
      temp_copy[2] = s1[0]*s2[1] - s1[1]*s2[0];
      d[0] = temp_copy[0];
      d[1] = temp_copy[1];
      d[2] = temp_copy[2];
    }
    break;
  default:
    {
    }
  }

  return d;
}

bool ise_v_equals  ( uint32_t vec_length, float* s1, float* s2 )
{
  uint32_t i;
  bool result = true;
  for( i=0; i<vec_length; i++, s1++, s2++ ) if( *s1 != *s2 ) result = false;
  return result;
}

float ise_v_dot  ( uint32_t vec_length, float* s1, float* s2 )
{
  uint32_t i;
  float result = 0.0f;
  for( i=0; i<vec_length; i++, s1++, s2++ ) result += *s1 * *s2;
  return result;
}

float* ise_v_min ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float* dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = (*s1 < *s2) ? *s1 : *s2;
  return d;
}

float* ise_v_max ( uint32_t vec_length, float* d, float* s1, float* s2 )
{
  uint32_t i;
  float* dp = d;
  for( i=0; i<vec_length; i++, dp++, s1++, s2++ ) *dp = (*s1 > *s2) ? *s1 : *s2;
  return d;
}

//  -----------------------------------------------------------------------------
//  Single argument matrix functions

float* ise_m_shrink( uint32_t rows, uint32_t cols, float* d, float* s, uint32_t remove_row, uint32_t remove_col )
{
  uint32_t srow, scol, drow, dcol;
  float* dptr = d;

  srow = (remove_row + 1);
  scol = (remove_col + 1);
  for( drow=0; drow<rows-1; drow++ ) {
    if( srow >= rows ) srow = 0;
    for( dcol=0; dcol<cols-1; dcol++ ) {
      if( scol >= cols ) scol = 0;
      *dptr = s[ (srow*cols) + scol ];
      dptr++;
      scol++;
    }
    srow++;
  }

  return d;
}

float  ise_m_determinant( uint32_t rows, float* s )
{
  switch( rows ) {
  case 2: return s[0]*s[3] - s[1]*s[2];
  case 3: return ( s[0]*( s[4]*s[8] - s[5]*s[7] ) +
                   s[1]*( s[5]*s[6] - s[3]*s[8] ) +
                   s[2]*( s[3]*s[7] - s[4]*s[6] ) );
  case 4: return ( s[0]*( s[5]*(s[10]*s[15] - s[11]*s[14]) +
                          s[6]*(s[11]*s[13] - s[9] *s[15]) +
                          s[7]*(s[9] *s[14] - s[10]*s[13]) ) +
                   s[1]*( s[6]*(s[11]*s[12] - s[8] *s[15]) +
                          s[7]*(s[8] *s[14] - s[10]*s[12]) +
                          s[4]*(s[10]*s[15] - s[11]*s[14]) ) +
                   s[2]*( s[7]*(s[8] *s[13] - s[9] *s[12]) +
                          s[4]*(s[9] *s[15] - s[11]*s[13]) +
                          s[5]*(s[11]*s[12] - s[8] *s[15]) ) +
                   s[3]*( s[4]*(s[9] *s[14] - s[10]*s[13]) +
                          s[5]*(s[10]*s[12] - s[8] *s[14]) +
                          s[6]*(s[8] *s[13] - s[9] *s[12]) ) );
  default: {
    float* temp_mat = new float[ (rows-1)*(rows-1) ];
    float result = 0.0;
    uint32_t i;
    for( i=0; i<rows; i++ ) {
      if( s[i] != 0.0f ) {
        ise_m_shrink( rows, rows, temp_mat, s, 0, i );
        result += s[i] * ise_m_determinant( rows-1, temp_mat );
      }
    }
    delete [] temp_mat;
    return result;
  }
  }
}

float* ise_m_transpose  ( uint32_t rows, uint32_t cols, float* d )
{
  uint32_t r, c;
  if( rows == cols ) {
    float temp;
    for( r=0; r<rows; r++ ) {
      for( c=r+1; c<cols; c++ ) {
        temp = d[ r*cols+c ];
        d[ r*cols+c ] = d[ c*rows+r ];
        d[ c*rows+r ] = temp;
      }
    }
  } else {
    float* temp_matrix = new float[ rows*cols ];
    for( r=0; r<rows; r++ ) {
      for( c=0; c<cols; c++ ) {
        temp_matrix[ c*rows+r ] = d[ r*cols+c ];
      }
    }
    memcpy( d, temp_matrix, rows*cols*sizeof(float) );
    delete [] temp_matrix;
  }
  return d;
}

float* ise_m_inverse    ( uint32_t rows, float* d )
{
  switch( rows ) {
  case 2:
    {
      float copy[4];
      copy[0] = d[0]; copy[1] = d[1]; copy[2] = d[2]; copy[3] = d[3];
      d[0] =  copy[3];
      d[1] = -copy[2];
      d[2] = -copy[1];
      d[3] =  copy[0];
      float det = ise_m_determinant( 2, d );
      if( det != 0 ) {
        det = 1.0f/det;
      }
      ise_sv_mul( 4, d, det, d );
    }
    break;
  case 3:
    {
      float copy[9];
      copy[0] = d[4]*d[8] - d[5]*d[7];
      copy[1] = d[5]*d[6] - d[3]*d[8];
      copy[2] = d[3]*d[7] - d[4]*d[6];

      copy[3] = d[2]*d[7] - d[1]*d[8];
      copy[4] = d[0]*d[8] - d[2]*d[6];
      copy[5] = d[1]*d[6] - d[0]*d[7];

      copy[6] = d[1]*d[5] - d[2]*d[4];
      copy[7] = d[2]*d[3] - d[0]*d[5];
      copy[8] = d[0]*d[4] - d[1]*d[3];

      float det = ise_v_dot( 3, d, copy );
      if( det != 0 ) {
        det = 1.0f/det;
      }
      ise_m_transpose( 3, 3, copy );
      ise_sv_mul( 9, d, det, copy );
    }
    break;
  case 4:
    {
      float copy[16];
      float tmp[12];
      float det;

      uint32_t r, c;
      for( r=0; r<4; r++ ) {
        for( c=0; c<4; c++ ) {
          copy[ r*rows+c ] = d[ c*rows+r ];
        }
      }

      // calculate pairs for first 8 elements (cofactors)
      tmp[0]  = copy[10] * copy[15];
      tmp[1]  = copy[11] * copy[14];
      tmp[2]  = copy[ 9] * copy[15];
      tmp[3]  = copy[11] * copy[13];
      tmp[4]  = copy[ 9] * copy[14];
      tmp[5]  = copy[10] * copy[13];
      tmp[6]  = copy[ 8] * copy[15];
      tmp[7]  = copy[11] * copy[12];
      tmp[8]  = copy[ 8] * copy[14];
      tmp[9]  = copy[10] * copy[12];
      tmp[10] = copy[ 8] * copy[13];
      tmp[11] = copy[ 9] * copy[12];

      // calculate first 8 elements (cofactors)
      d[0]  = tmp[0]*copy[5] + tmp[3]*copy[6] + tmp[4] *copy[7];
      d[0] -= tmp[1]*copy[5] + tmp[2]*copy[6] + tmp[5] *copy[7];
      d[1]  = tmp[1]*copy[4] + tmp[6]*copy[6] + tmp[9] *copy[7];
      d[1] -= tmp[0]*copy[4] + tmp[7]*copy[6] + tmp[8] *copy[7];
      d[2]  = tmp[2]*copy[4] + tmp[7]*copy[5] + tmp[10]*copy[7];
      d[2] -= tmp[3]*copy[4] + tmp[6]*copy[5] + tmp[11]*copy[7];
      d[3]  = tmp[5]*copy[4] + tmp[8]*copy[5] + tmp[11]*copy[6];
      d[3] -= tmp[4]*copy[4] + tmp[9]*copy[5] + tmp[10]*copy[6];
      d[4]  = tmp[1]*copy[1] + tmp[2]*copy[2] + tmp[5] *copy[3];
      d[4] -= tmp[0]*copy[1] + tmp[3]*copy[2] + tmp[4] *copy[3];
      d[5]  = tmp[0]*copy[0] + tmp[7]*copy[2] + tmp[8] *copy[3];
      d[5] -= tmp[1]*copy[0] + tmp[6]*copy[2] + tmp[9] *copy[3];
      d[6]  = tmp[3]*copy[0] + tmp[6]*copy[1] + tmp[11]*copy[3];
      d[6] -= tmp[2]*copy[0] + tmp[7]*copy[1] + tmp[10]*copy[3];
      d[7]  = tmp[4]*copy[0] + tmp[9]*copy[1] + tmp[10]*copy[2];
      d[7] -= tmp[5]*copy[0] + tmp[8]*copy[1] + tmp[11]*copy[2];

      // calculate pairs for second 8 elements (cofactors)
      tmp[0]  = copy[2]*copy[7];
      tmp[1]  = copy[3]*copy[6];
      tmp[2]  = copy[1]*copy[7];
      tmp[3]  = copy[3]*copy[5];
      tmp[4]  = copy[1]*copy[6];
      tmp[5]  = copy[2]*copy[5];
      tmp[6]  = copy[0]*copy[7];
      tmp[7]  = copy[3]*copy[4];
      tmp[8]  = copy[0]*copy[6];
      tmp[9]  = copy[2]*copy[4];
      tmp[10] = copy[0]*copy[5];
      tmp[11] = copy[1]*copy[4];

      // calculate second 8 elements (cofactors)
      d[ 8]  = tmp[0] *copy[13] + tmp[3] *copy[14] + tmp[4] *copy[15];
      d[ 8] -= tmp[1] *copy[13] + tmp[2] *copy[14] + tmp[5] *copy[15];
      d[ 9]  = tmp[1] *copy[12] + tmp[6] *copy[14] + tmp[9] *copy[15];
      d[ 9] -= tmp[0] *copy[12] + tmp[7] *copy[14] + tmp[8] *copy[15];
      d[10]  = tmp[2] *copy[12] + tmp[7] *copy[13] + tmp[10]*copy[15];
      d[10] -= tmp[3] *copy[12] + tmp[6] *copy[13] + tmp[11]*copy[15];
      d[11]  = tmp[5] *copy[12] + tmp[8] *copy[13] + tmp[11]*copy[14];
      d[11] -= tmp[4] *copy[12] + tmp[9] *copy[13] + tmp[10]*copy[14];
      d[12]  = tmp[2] *copy[10] + tmp[5] *copy[11] + tmp[1] *copy[ 9];
      d[12] -= tmp[4] *copy[11] + tmp[0] *copy[ 9] + tmp[3] *copy[10];
      d[13]  = tmp[8] *copy[11] + tmp[0] *copy[ 8] + tmp[7] *copy[10];
      d[13] -= tmp[6] *copy[10] + tmp[9] *copy[11] + tmp[1] *copy[ 8];
      d[14]  = tmp[6] *copy[ 9] + tmp[11]*copy[11] + tmp[3] *copy[ 8];
      d[14] -= tmp[10]*copy[11] + tmp[2] *copy[ 8] + tmp[7] *copy[ 9];
      d[15]  = tmp[10]*copy[10] + tmp[4] *copy[ 8] + tmp[9] *copy[ 9];
      d[15] -= tmp[8] *copy[ 9] + tmp[11]*copy[10] + tmp[5] *copy[ 8];

      // calculate determinant
      det = ise_v_dot( 4, copy, d );
      if( det != 0 ) {
        det = 1/det;
      }
      ise_sv_mul( 16, d, det, d );
    }
    break;
  default:
    {
    }
  }

  return d;
}

float* ise_m_swizzle    ( uint32_t rows, uint32_t cols, float* d, uint32_t* row_indices, uint32_t* col_indices )
{
  uint32_t i, len = rows*cols;
  float static_copy[ISE_M_STATIC_ARRAY_SIZE];
  float *temp_copy;

  if( len > ISE_M_STATIC_ARRAY_SIZE ) {
    temp_copy = new float[len];
  } else {
    temp_copy = static_copy;
  }

  for( i=0; i<len; i++ ) {
    temp_copy[i] = d[ row_indices[i]*cols + col_indices[i] ];
  }

  if( temp_copy != static_copy ) {
    memcpy( d, temp_copy, len );
    delete [] temp_copy;
  } else {
    for( i=0; i<len; i++ ) d[i] = temp_copy[i];
  }

  return d;
}

float* ise_m_set_identity( uint32_t rows, float* d )
{
  uint32_t r, c;
  float* dptr = d;

  for( r=0; r<rows; r++ ) {
    for( c=0; c<rows; c++ ) {
      *dptr = (r == c) ? 1.0f : 0.0f;
      dptr++;
    }
  }

  return d;
}

float* ise_m_set_rotation( uint32_t rows, float* d, float angle, float* axis )
{
  float cos_ang = cos(angle);
  float sin_ang = sin(angle);

  switch(rows) {
  case 2:
    d[0] =  cos_ang;
    d[1] = -sin_ang;
    d[2] =  sin_ang;
    d[3] =  cos_ang;
    break;
  case 3:
    d[0] = cos_ang           + (1-cos_ang) * axis[0] * axis[0];
    d[1] = sin_ang * axis[2] + (1-cos_ang) * axis[0] * axis[1];
    d[2] =-sin_ang * axis[1] + (1-cos_ang) * axis[0] * axis[2];

    d[3] =-sin_ang * axis[2] + (1-cos_ang) * axis[1] * axis[0];
    d[4] = cos_ang           + (1-cos_ang) * axis[1] * axis[1];
    d[5] = sin_ang * axis[0] + (1-cos_ang) * axis[1] * axis[2];

    d[6] = sin_ang * axis[1] + (1-cos_ang) * axis[2] * axis[0];
    d[7] =-sin_ang * axis[0] + (1-cos_ang) * axis[2] * axis[1];
    d[8] = cos_ang           + (1-cos_ang) * axis[2] * axis[2];
    break;
  case 4:
    d[ 0] = cos_ang           + (1-cos_ang) * axis[0] * axis[0];
    d[ 1] = sin_ang * axis[2] + (1-cos_ang) * axis[0] * axis[1];
    d[ 2] =-sin_ang * axis[1] + (1-cos_ang) * axis[0] * axis[2];
    d[ 3] = 0.0f;

    d[ 4] =-sin_ang * axis[2] + (1-cos_ang) * axis[1] * axis[0];
    d[ 5] = cos_ang           + (1-cos_ang) * axis[1] * axis[1];
    d[ 6] = sin_ang * axis[0] + (1-cos_ang) * axis[1] * axis[2];
    d[ 7] = 0.0f;

    d[ 8] = sin_ang * axis[1] + (1-cos_ang) * axis[2] * axis[0];
    d[ 9] =-sin_ang * axis[0] + (1-cos_ang) * axis[2] * axis[1];
    d[10] = cos_ang           + (1-cos_ang) * axis[2] * axis[2];
    d[11] = 0.0f;

    d[12] = 0.0f;
    d[13] = 0.0f;
    d[14] = 0.0f;
    d[15] = 1.0f;
    break;
  }

  return d;
}

//  -----------------------------------------------------------------------------
//  Two argument matrix functions

float* ise_m_mul( uint32_t s1_rows, uint32_t s2_rows, uint32_t s2_cols, float* d, float* s1, float* s2 )
{
  float static_copy1[ISE_M_STATIC_ARRAY_SIZE];
  float static_copy2[ISE_M_STATIC_ARRAY_SIZE];
  float *s1ptr;
  float *s2ptr;
  float *dptr;
  uint32_t s1size = s1_rows*s2_rows;
  uint32_t s2size = s2_rows*s2_cols;
  uint32_t r, c;
  float *v1;
  float *v2;

  // if s1 == d, then make a copy of s1
  if( s1 == d ) {
    if( s1size > ISE_M_STATIC_ARRAY_SIZE ) {
      s1ptr = new float[s1size];
      memcpy( s1ptr, s1, s1size );
    } else {
      s1ptr = static_copy1;
      for( r=0; r<s1size; r++ ) s1ptr[r] = s1[r];
    }
  } else {
    s1ptr = s1;
  }

  // Make a tranposed copy of s2
  if( s2size > ISE_M_STATIC_ARRAY_SIZE ) {
    s2ptr = new float[s2size];
  } else {
    s2ptr = static_copy2;
  }
  memcpy( s2ptr, s2, s2size*sizeof(float) );
  ise_m_transpose( s2_rows, s2_cols, s2ptr );

  // Loop through every row of s1, and coumn of s2,
  // and take the dot product of the row/column pair, and assign to it the
  // (rth, cth) entry of the destination matrix
  dptr = d;
  for( r=0, v1 = s1ptr; r<s1_rows; r++, v1 += s2_rows ) {
    for( c=0, v2 = s2ptr; c<s2_cols; c++, v2 += s2_rows ) {
      *dptr = ise_v_dot( s2_rows, v1, v2 );
      dptr++;
    }
  }

  // Free any memory that was allocated
  if( (s1ptr != s1) && (s1ptr != static_copy1) ) delete [] s1ptr;
  if( s2ptr != static_copy2 ) delete [] s2ptr;

  return d;
}

//  -----------------------------------------------------------------------------
//  Functions that set 4x4 matrices

float* ise_m4_set_perspective_off_center( float* d, float left, float right, float bottom, float top, float znear, float zfar, bool left_handed, bool dx_style )
{
  d[ 0] = 2.0f*znear/(right-left);
  d[ 1] = 0.0f;
  d[ 2] = (left+right)/(right-left);
  d[ 3] = 0.0f;

  d[ 4] = 0.0f;
  d[ 5] = 2.0f*znear/(top-bottom);
  d[ 6] = (bottom+top)/(top-bottom);
  d[ 7] = 0.0f;

  d[ 8] = 0.0f;
  d[ 9] = 0.0f;
  if( dx_style ) {
    d[10] = zfar/(znear-zfar);
    d[11] = znear*zfar/(znear-zfar);
  } else {
    d[10] = (zfar+znear)/(znear-zfar);
    d[11] = (2.0f*znear*zfar)/(znear-zfar);
  }

  d[12] = 0.0f;
  d[13] = 0.0f;
  d[14] = -1.0f;
  d[15] = 0.0f;

  if( left_handed ) {
    d[ 2] = -d[ 2];
    d[ 6] = -d[ 6];
    d[10] = -d[10];
    d[14] = -d[14];
  }
  return d;
}

float* ise_m4_set_perspective_fov( float* d, float fovy, float aspect, float znear, float zfar, bool left_handed, bool dx_style )
{
  float tan_f = tan( fovy/2.0f );

  d[ 0] = 1.0f/(aspect*tan_f);
  d[ 1] = 0.0f;
  d[ 2] = 0.0f;
  d[ 3] = 0.0f;

  d[ 4] = 0.0f;
  d[ 5] = 1.0f/tan_f;
  d[ 6] = 0.0f;
  d[ 7] = 0.0f;

  d[ 8] = 0.0f;
  d[ 9] = 0.0f;
  if( dx_style ) {
    d[10] = zfar/(znear-zfar);
    d[11] = znear*zfar/(znear-zfar);
  } else {
    d[10] = (zfar+znear)/(znear-zfar);
    d[11] = (2.0f*znear*zfar)/(znear-zfar);
  }

  d[12] = 0.0f;
  d[13] = 0.0f;
  d[14] = -1.0f;
  d[15] = 0.0f;

  if( left_handed ) {
    d[10] = -d[10];
    d[14] = -d[14];
  }

  return d;
}

float* ise_m4_set_ortho_off_center( float* d, float left, float right, float bottom, float top, float znear, float zfar, bool left_handed, bool dx_style )
{
  d[ 0] = 2.0f/(right-left);
  d[ 1] = 0.0f;
  d[ 2] = 0.0f;
  d[ 3] = (left+right)/(left-right);

  d[ 4] = 0.0f;
  d[ 5] = 2.0f/(top-bottom);
  d[ 6] = 0.0f;
  d[ 7] = (bottom+top)/(bottom-top);

  d[ 8] = 0.0f;
  d[ 9] = 0.0f;
  if( dx_style ) {
    d[10] = 1.0f/(znear-zfar);
    d[11] = znear/(znear-zfar);
  } else {
    d[10] = 2.0f/(znear-zfar);
    d[11] = (znear+zfar)/(znear-zfar);
  }

  d[12] = 0.0f;
  d[13] = 0.0f;
  d[14] = 0.0f;
  d[15] = 1.0f;

  if( left_handed )
    d[10] = -d[10];

  return d;
}

float* ise_m4_set_look_at( float* d, float* eye, float* at, float* up_dir, bool left_handed )
{
  float *xaxis = &(d[0]), *yaxis = &(d[4]), *zaxis = &(d[8]);

  if( left_handed ) {
    ise_v3_normalize( ise_v3_sub( zaxis, at, eye ) );
  } else {
    ise_v3_normalize( ise_v3_sub( zaxis, eye, at ) );
  }
  ise_v3_normalize( ise_v3_cross( xaxis, up_dir, zaxis ) );
  ise_v3_cross( yaxis, zaxis, xaxis );

  d[ 3] = -ise_v3_dot( xaxis, eye );
  d[ 7] = -ise_v3_dot( yaxis, eye );
  d[11] = -ise_v3_dot( zaxis, eye );
  d[12] = 0.0f;
  d[13] = 0.0f;
  d[14] = 0.0f;
  d[15] = 1.0f;

  return d;
}
