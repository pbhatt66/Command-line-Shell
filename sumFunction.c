#include <stdio.h>
#include <stdlib.h>

// take input from a file using redirection, and print the sum of the numbers
int main(int argc, char *argv[]) {
    int sum = 0;
    int num;
    while (scanf("%d", &num) != EOF) {
        sum += num;
    }
    printf("%d\n", sum);
    return EXIT_SUCCESS;
}
