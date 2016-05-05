/**
 *
 * Description: Test tag checks for double word (64 bit), word (32 bit), half word (16 bit)
 * and byte (8 bit) access.
 *
 * Currently this only works if tags are not propagated from memory into registers and vice versa.
 */
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include"env/tag.h"

#define test_body(name, datatype, load_asm, store_asm) 		 	\
bool load_store_##name(void *location) {				\
	int i, limit = sizeof(uint64_t) / sizeof(datatype);		\
	for(i = 0; i < limit; i++) {					\
		datatype *pointer = (datatype *) location;		\
		pointer = pointer + i;					\
		datatype value = 32;					\
		asm volatile (	#load_asm" a4, 0(%0)\n"			\
				"not a4, a4\n"				\
				#store_asm"  a4, 0(%0)\n" 		\
				: : "r"(pointer) : "a4");		\
	}								\
	return true;							\
}		

test_body(unsigned_byte, uint8_t, lbu, sb)
test_body(signed_byte, int8_t, lb, sb)
test_body(unsigned_halfword, uint16_t, lhu, sh)
test_body(signed_halfword, int16_t, lh, sh)
test_body(unsigned_word, uint32_t, lwu, sw)
test_body(signed_word, int32_t, lw, sw)
test_body(unsigned_long, uint64_t, ld, sd) 

bool load_store_checkrun(void *test_memory, bool expect_load_trap, bool expect_store_trap) {

	int load_traps = expect_load_trap ? 1 : 0;
	int store_traps = expect_store_trap ? 1 : 0;

	printf("-- load and store data 64bit, expect %d load and %d store traps\n",
		load_traps, store_traps);
	load_store_unsigned_long(test_memory);
	printf("-- load and store unsigned data 32bit, expect %d load and %d store traps\n",
		load_traps*2, store_traps*2);
	load_store_unsigned_word(test_memory);
	printf("-- load and store signed data 32bit, expect %d load and %d store traps\n",
		load_traps*2, store_traps*2);
	load_store_signed_word(test_memory);
	printf("-- load and store unsigned data 16bit, expect %d load and %d store traps\n",
		load_traps*4, store_traps*4);
	load_store_unsigned_halfword(test_memory);
	printf("-- laod and store signed data 16bit, expect %d load and %d store traps\n",
		load_traps*4, store_traps*4);
	load_store_signed_halfword(test_memory);
	printf("-- load and store unsigned data 8bit, expect %d load and %d store traps\n",
		load_traps*8, store_traps*8);
	load_store_unsigned_byte(test_memory);
	printf("-- load and store signed data 8bit, expect %d load and %d store traps\n",
		load_traps*8, store_traps*8);
	load_store_signed_byte(test_memory);

	return true;
}

bool load_store_check() {
	
	// create memory
	long* test_memory = (long *) malloc(sizeof(long));
	if(test_memory == NULL) {
		fprintf(stderr, "Error - could not allocate enough memory!\n");
		return false;
	}
	printf("Memory allocated at %p\n", test_memory);
	// tag memory location
	*test_memory = 0xDEADBEEFDEADBEEFL;
	store_tag(test_memory, 1);
	// set tag behaviour

	printf(" ---- Test load and store traps ----\n");
	read_write_load_csr(0b10); 	// trap on value 0x01
	read_write_store_csr(0b10); 	// trap on value 0x01
	load_store_checkrun(test_memory, true, true);

	printf(" ---- Test no traps ----\n");
	read_write_load_csr(0b100);
	read_write_store_csr(0b100);
	load_store_checkrun(test_memory, false, false);

	printf(" ---- Test only load traps ----\n");
	read_write_load_csr(0b110);
	load_store_checkrun(test_memory, true, false);

	printf(" ---- Test only store traps ----\n");
	read_write_load_csr(0b100);
	read_write_store_csr(0b110);
	load_store_checkrun(test_memory, false, true);

	// reset CSRs:
	read_write_load_csr(0);
	read_write_store_csr(0);

}

int main (int argc, char **argv) {
	bool success = true;
	success = success && load_store_check();
	if (success) 
		printf("All tests passed!\n");
}
