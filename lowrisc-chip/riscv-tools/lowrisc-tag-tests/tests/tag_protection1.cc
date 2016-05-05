// See LICENSE for license details.

#include "env/tag.h"
#include <stdio.h>
#include <stdlib.h>

#define VECT_SIZE 1<<9

int main() {

  long a[VECT_SIZE], i;
  
  printf("Testing for %d iterations\n", VECT_SIZE);
  for(i=0;i<VECT_SIZE;i++) {
    store_tag(a+i, 1); // Read only.
  }
  for(i=0;i<VECT_SIZE;i++) {
    int t = load_tag(a+i);
    if(t != 1) {
       printf("Tag is not 1 for %i\n", i);
    }
  }
  for(i=0;i<VECT_SIZE;i++) {
    printf("Read %ld\n", a[i]);
    printf("Should fail on write...\n");
    a[i] = 5;
    printf("TEST FAILED!\n");
    return 1;
  }
  return 0;
}
