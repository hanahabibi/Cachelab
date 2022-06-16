/*H******************************************************
 * Group members:
 * Noemi Kallweit
 * Hana Habibi
*H*/

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";

void transposeXX(int blocksize, int M, int N, int A[N][M], int B[M][N])
{
    // loop variables
	int i, j, k, l;
    //coordinate and value of the diagnoal
    int d, dv = 0;

    //seperate the matrix into blocks of bloacksize
	for(i = 0; i < N; i+=blocksize){
		for(j = 0; j < M; j+=blocksize){
            //iterate through the blocks
			for(k = j; k < (j+blocksize); k++){
				for(l = i; l < (i+blocksize); l++){
						
                    if(k!=l) {
                        //transpose
                        B[l][k] = A[k][l];
                    }
                    else{
                        //save value and coordinate of the diagonal element
                        d = k;
                        dv = A[l][k];
                    }

                }
                //assign the diagonal value
				if(i == j){
				B[d][d] = dv;
				}	
			}
		}
	}
}

void transposeXY(int blocksize, int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l;
    int d, dv = 0;

    //seperate the matrix into blocks of bloacksize
	for(i = 0; i < M; i+=blocksize){
        for(j = 0; j < N; j+=blocksize){
            //iterate through the blocks
            for(k = j; k < (j+blocksize) && (k < N) ; k++){
                for(l = i; l < (i+blocksize) && (l < M); l++){

                    if(k!=l) {
                        //transpose
                        B[l][k] = A[k][l];
                    }
                    else{
                        //save value and coordinate of the diagonal element
                        d = k;
                        dv = A[l][k];
                    }
                }
                if(i == j){
                    //assign the diagonal value
                    B[d][d] = dv;
                }
            }
        }
	}

}

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    //if N and M are 32 set bloack size to 8
	if(N == 32) {
		transposeXX(8, M, N, A, B);
    }
    //if N and M are 64 set blocksize to 4
	else if(N == 64){
		transposeXX(4, M, N, A, B);
    }
    //if N is 67 and M 61
	else {
        transposeXY(16, M, N, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

