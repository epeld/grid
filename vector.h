#ifndef GRID_VECTOR_H
#define GRID_VECTOR_H

#include <math.h>

typedef float gridfloat;

typedef struct {
	gridfloat x,y,z;
} GRID_Vector;


static inline GRID_Vector	GRID_vectorNegate( GRID_Vector v ){ GRID_Vector v2 = {-v.x, -v.y, -v.z}; return v2; }
static inline GRID_Vector	GRID_vectorAdd( GRID_Vector v, GRID_Vector v2 ){ GRID_Vector v3 = { v.x + v2.x, v.y + v2.y, v.z + v2.z }; return v3; }
static inline GRID_Vector	GRID_vectorCross( GRID_Vector v, GRID_Vector v2 ){ GRID_Vector v3 = {v.y * v2.z - v.z * v2.y, v.z * v2.x - v.x * v2.z, v.x * v2.y - v.y * v2.x }; return v3;}
static inline GRID_Vector	GRID_vectorScale( GRID_Vector v, gridfloat k ){ GRID_Vector v2 = { k*v.x, k*v.y, k*v.z }; return v2; }

static inline gridfloat	GRID_vectorInner( GRID_Vector v, GRID_Vector v2 ){ return v.x * v2.x + v.y * v2.y + v.z * v2.z; }
static inline gridfloat	GRID_vectorMagnitude( GRID_Vector v ){ return sqrt( GRID_vectorInner( v, v ) ); }
static inline gridfloat GRID_vectorMagnitudeSquared( GRID_Vector v ) {return GRID_vectorInner( v, v ); }


#define		GRID_glVector( v ) glVertex3f( (v).x, (v).y, (v).z )
#define 	GRID_ZEROVECTOR ( (GRID_Vector){0,0,0} )

#define GRID_vectorSubtract( v1, v2 ) ( GRID_vectorAdd( v1, GRID_vectorNegate( v2 ) ) )

#endif
