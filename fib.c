#include <stdio.h>

int nthFibonacci(int n){
    if (n <= 1){
        return n;
    }
    return nthFibonacci(n - 1) + nthFibonacci(n - 2);
}

int main(){
    int n;
    printf("Enter the number: \n");
    scanf("%d", &n);
    int result = nthFibonacci(n);
    printf("%d\n", result);
    return 0;
}
