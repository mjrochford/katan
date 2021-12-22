#include <stdio.h>
#include <assert.h>

#include "rules.h"
extern void printGameBoard();

int main(void) {
    initalizeGameBoard();
    printGameBoard();
    return 1;
}
