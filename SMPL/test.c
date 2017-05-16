#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int numberOfDigits(int n) 
{
    if (n == 0)
        return 0;
    return floor( log10( abs( n ) ) ) + 1;
}
void main(){
    printf("%d\t%d\t%d\t%d\t%d\n",numberOfDigits(1),numberOfDigits(10),numberOfDigits(100),numberOfDigits(1000),numberOfDigits(10000));
}