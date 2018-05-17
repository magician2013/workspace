/**
 * @file types.hh
 * @brief functions related with some vector types
 * @author Yu TAKAHASHI<yuu-t@is.naist.jp>
 * 
 * @date 2007-12-19
 *
 */
/*
	Copyright (c) 2008 Yu Takahashi, Shikano-Lab., Nara Institute of Science and Technology (NAIST)
*/
#include "types.hh"

/** Initialize 3-D vector (allocate memory as obj[size1][size2][size3])
 * @param obj   target object
 * @param size1 size 1
 * @param size2 size 2
 * @param size3 size 3
 */
void init_3d( C3_VECTOR &obj , int size1 , int size2 , int size3 )
{
        int i,j;
        
        obj.resize(size1);
        for(i=0;i<size1;i++) {
                obj[i].resize(size2);
                for(j=0;j<size2;j++){
                        obj[i][j].resize(size3);
                }
        }
}

void init_3d( D3_VECTOR &obj , int size1 , int size2 , int size3 )
{
        int i,j;
        
        obj.resize(size1);
        for(i=0;i<size1;i++) {
                obj[i].resize(size2);
                for(j=0;j<size2;j++){
                        obj[i][j].resize(size3);
                }
        }
}
