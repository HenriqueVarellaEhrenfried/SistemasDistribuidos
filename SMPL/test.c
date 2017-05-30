#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int lastIndex(int * array){
    int i=0;
    while (*(array + i)){
        puts("oi");
        printf("array[%d]: %d\n",i,*(array + i));
        i++;
    }
}
void main(){
    int a[10];
    lastIndex(a);
}