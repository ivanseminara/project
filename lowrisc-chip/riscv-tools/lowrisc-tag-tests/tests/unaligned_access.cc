/**
 * Description: This simply tests the behaviour of Spike for unaligned accesses of 64bit words.
 *
 * TODO: check what happens on unaligned access when one of the memory locations is tagged!
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<env/tag.h>


int main(int argc, char **argv) {

	long value = 0xbeefbeefbeefbeef;
	uint64_t *pointer = (uint64_t *) malloc(sizeof(uint64_t) * 2);

	pointer[0] = value;
	pointer[1] = 0xdeaddeaddeaddead;
	store_tag(pointer, 1);
	store_tag(pointer+1, 2);

	printf("initial pointer: %p\n", pointer);
	uint8_t *bytepointer = (uint8_t *) pointer;
	char tag;

	for(int i=0; i <= 8; i++) {
		uint64_t *unaligned_pointer = (uint64_t *)bytepointer;
		value = *unaligned_pointer;
		tag = load_tag(unaligned_pointer);
	
		printf("unaligned pointer: %p\n", unaligned_pointer);
		printf("value at unaligned address: 0x%lx\n", value);
		printf("tag at unaligned address: %d\n", tag);
		bytepointer++;
	}
	
}
