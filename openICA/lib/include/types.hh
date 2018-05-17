/**
 * @file types.hh
 * @brief definitions about complex, some vectors
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#ifndef _TYPES_H_INCLUDED_
#define _TYPES_H_INCLUDED_

#include <cmath>
#include <vector>
#include <new>

#include "config.hh"

/*! \struct COMPLEX
 *  \brief complex value structure
 */
typedef struct {
        //! Real part
        FLOAT re;
        //! Imaginary part
        FLOAT im;
} COMPLEX;

//! vector of int
typedef std::vector<int> I_VECTOR;
//! 2D vector of int
typedef std::vector< std::vector<int> > I2_VECTOR;
//! vector of short
typedef std::vector< short > S_VECTOR;
//! 2D vector of short
typedef std::vector< std::vector<short> > S2_VECTOR;

//! vector of FLOAT
typedef std::vector<FLOAT> D_VECTOR;
//! 2D vector of FLOAT
typedef std::vector< std::vector<FLOAT> >  D2_VECTOR;
//! 3D vector of FLOAT
typedef std::vector< std::vector< std::vector<FLOAT> > > D3_VECTOR;
//! vector of COMPLEX
typedef std::vector<COMPLEX> C_VECTOR;
//! 2D vector of COMPLEX
typedef std::vector< std::vector<COMPLEX> >  C2_VECTOR;
//! 3D vector of COMPLEX
typedef std::vector< std::vector< std::vector<COMPLEX> > > C3_VECTOR;

/**********************************************************************/
extern void init_3d( C3_VECTOR &obj , int size1 , int size2 , int size3 );
extern void init_3d( D3_VECTOR &obj , int size1 , int size2 , int size3 );


#endif
/* end of types.hh */
