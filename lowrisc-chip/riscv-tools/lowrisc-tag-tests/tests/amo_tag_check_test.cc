/**
 *
 * Test tag checks for LR/SC double word and word access, and atomic memory
 * operations (both double word and word).
 *
 * Currently these tests only execute correctly when tags are not automatically
 * copied from memory into register tags and vice versa.
 */
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include"env/tag.h"

#define test_body(datatype, load_asm, store_asm, location) ({ 		\
	int i, limit = sizeof(uint64_t) / sizeof(datatype);		\
	for(i = 0; i < limit; i++) {					\
		datatype *pointer = (datatype *) location;		\
		pointer = pointer + i;					\
		datatype value = 32;					\
		asm volatile (	#load_asm" a4, 0(%0)\n"			\
				"not a4, a4\n"				\
				#store_asm"  a3 ,a4, 0(%0)\n" 		\
				: : "r"(pointer) : "a4", "a3");		\
	} })

#define amo_test(name, datatype, amo_asm)				\
bool amo_##name(void *location, datatype rs2) {				\
	int i, limit = sizeof(uint64_t) / sizeof(datatype);		\
	for(i=0; i < limit; i++) {					\
		datatype *pointer = (datatype *) location;		\
		pointer = pointer + i;					\
		datatype value = 32;					\
		asm volatile ( #amo_asm" %0, %1, (%2)\n"		\
				: "=r"(value)				\
				: "r"(rs2), "r"(pointer) ); 		\
	}								\
	return true;							\
} 

bool lrsc_long(void *location) {
	test_body(uint64_t, lr.d, sc.d, location);
	return true;
}
bool lrsc_int(void *location) {
	test_body(uint32_t, lr.w, sc.w, location);
	return true;
}

amo_test(swap_word, uint32_t, amoswap.w);
amo_test(swap_doubleword, uint64_t, amoswap.d);
amo_test(add_word, uint32_t, amoadd.w)
amo_test(add_doubleword, uint64_t, amoadd.d)
amo_test(xor_word, uint32_t, amoxor.w)
amo_test(xor_doubleword, uint64_t, amoxor.d)
amo_test(and_word, uint32_t, amoand.w)
amo_test(and_doubleword, uint64_t, amoand.d)
amo_test(or_word, uint32_t, amoor.w)
amo_test(or_doubleword, uint64_t, amoxor.d)
amo_test(min_word, uint32_t, amomin.w)
amo_test(min_doubleword, uint64_t, amomin.d)
amo_test(minu_word, uint32_t, amominu.w)
amo_test(minu_doubleword, uint64_t, amominu.d)
amo_test(max_word, uint32_t, amomax.w)
amo_test(max_doubleword, uint64_t, amomax.d)
amo_test(maxu_word, uint32_t, amomaxu.w)
amo_test(maxu_doubleword, uint64_t, amomaxu.d)

bool load_store_checkrun(void *test_memory, bool expect_load_trap, bool expect_store_trap) {
	uint64_t doubleword_rs2 = 2;
	uint32_t word_rs2 = 2;

	//tests:
	int load_traps = expect_load_trap ? 1 : 0;
	int store_traps = (expect_store_trap) ? 1 : 0;
	printf("-- LRSC test 64bit, expect %d load and %d store traps\n", load_traps, store_traps);
	lrsc_long(test_memory);
	printf("-- LRSC test 32bit, expect %d load and %d store traps\n", load_traps*2, store_traps*2);
	lrsc_int(test_memory);

	store_traps = (!expect_load_trap && expect_store_trap) ? 1 : 0;
	
	printf("-- AMO swap test 64bit, expect %d load and %d store traps\n",
		load_traps, store_traps);
	amo_swap_doubleword(test_memory, doubleword_rs2);
	printf("-- AMO swap test 32bit, expect %d load and %d store traps\n", 
		load_traps*2, store_traps*2);
	amo_swap_word(test_memory, word_rs2);

	printf("-- remaining AMO tests 64bit, expect %d load and %d store traps\n",
		load_traps*8, store_traps*8);
	amo_add_doubleword(test_memory, doubleword_rs2);
	amo_xor_doubleword(test_memory, doubleword_rs2);
	amo_and_doubleword(test_memory, doubleword_rs2);
	amo_or_doubleword(test_memory, doubleword_rs2);
	amo_min_doubleword(test_memory, doubleword_rs2);
	amo_minu_doubleword(test_memory, doubleword_rs2);
	amo_max_doubleword(test_memory, doubleword_rs2);
	amo_maxu_doubleword(test_memory, doubleword_rs2);

	printf("-- remaining AMO tests 32bit, expect %d load and %d store traps\n",
		load_traps*16, store_traps*16);
	amo_add_word(test_memory, word_rs2);
	amo_xor_word(test_memory, word_rs2);
	amo_and_word(test_memory, word_rs2);
	amo_or_word(test_memory, word_rs2);
	amo_min_word(test_memory, word_rs2);
	amo_minu_word(test_memory, word_rs2);
	amo_max_word(test_memory, word_rs2);
	amo_maxu_word(test_memory, word_rs2);

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

	char mem_tag = 1;
	*test_memory = 0xDEADBEEFDEADBEEF;
	store_tag(test_memory, mem_tag);

	printf(" ---- Test load and store traps ----\n");
        read_write_load_csr(0b10);      // trap on value 0x01
        read_write_store_csr(0b10);     // trap on value 0x01
	load_store_checkrun(test_memory, true, true);

	printf("---- Test no traps ----\n");
	read_write_load_csr(0b100);
	read_write_store_csr(0b100);
	load_store_checkrun(test_memory, false, false);

	printf("---- Test only load traps ----\n");
	read_write_load_csr(0b10);
	read_write_store_csr(0b100);
	load_store_checkrun(test_memory, true, false);

	printf("---- Test only store traps ----\n");
	read_write_load_csr(0b100);
	read_write_store_csr(0b10);
	load_store_checkrun(test_memory, false, true);

	// reset CSRs:
	read_write_load_csr(0);
	read_write_store_csr(0);

	return true;
}


int main (int argc, char **argv) {
	bool success = true;
	success = success && load_store_check();
	if (success) 
		printf("All tests passed!\n");
}
