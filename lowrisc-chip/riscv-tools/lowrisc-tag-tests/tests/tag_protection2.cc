// See LICENSE for license details.

#include "env/tag.h"
#include <stdio.h>
#include <stdlib.h>

#define VECT_SIZE 1<<9

int main() {

  long a[VECT_SIZE], i;
  
  store_tag(&a[0], 2); // Write only.
  a[0] = 5;
  printf("Should fail on read...\n");
  printf("Read %ld\n", a[0]);
  printf("TEST FAILED!\n");
}
