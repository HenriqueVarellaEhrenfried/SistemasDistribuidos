#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

createMat(int m[2][2]){
    int i,j;
    for(i=0;i<2;i++){
        for(j=0;j<2;j++){
            m[i][j] = i*2+j;
        }
    }
}

void main(){
    // int * mat;
    // int rows = 3, columns = 4, i, j;
    // mat = (int*)malloc(sizeof(int)*rows*columns);
    
    // for(i = 0; i < rows; i++){
    //     for(j = 0; j < columns; j++){
    //         printf("(%d %d) ",i,j);
    //     }
    //     printf("\n");
    // }
    // int N = 8;
    // tcis a[N][clusters];
    // int i,j,k,num_nodes;
    // int clusters =(int)log2(N);
    // for(i=0;i<N;i++){
    //     for(j=0;j<clusters;j++){
    //         num_nodes = pow(2,j);
    //         table_cis[i][j].cluster_id = j+1; 
    //         table_cis[i][j].cis = (int*)malloc(sizeof(int)*num_nodes);
    //         nodes = cis(i, j);
    //         for(k = 0; k < num_nodes; k++){
    //             table_cis[i][j].cis[k] = nodes[k];
    //         }
    //     }
    // }
    int mat[2][2];
    createMat(mat);
    int i,j;
    for(i=0;i<2;i++){
        for(j=0;j<2;j++){
            printf("%d ",mat[i][j]);
        }
        puts(" ");
    }
}