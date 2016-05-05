#include <stdio.h>

void function1() {
    printf("Function 2\n");
    printf("Insert value: ");
    char buffer[10];
    gets(buffer);
    printf("value: %s\n", buffer);
}

void function2() {
    printf("Function 3\n");
}

int main() {
    function1();
    void (*fp2)() = &function2;
    fp2();

    return 0;
}

