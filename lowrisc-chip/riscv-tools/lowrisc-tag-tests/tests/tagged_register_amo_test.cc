#include"env/tag.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define MIN(a,b) ((a < b) ? a : b)
#define MAX(a,b) ((a > b) ? a : b)

/**
 * This function checks whether atomic memory operations preserve memory/register tags
 * amoxx rd, rs1, rs2 loads the value from address in rs1 into rd, applies a 
 * binary operator (specified by xx) to rd and rs1, and stores the value back into the 
 * location specified by rs1
 */
bool check_amo(char *amo_ins) {
        // allocate memory
        long *ptr = (long *) malloc(sizeof(long));
        if (ptr == NULL) {
                fprintf(stderr, "Could not allocate enough memory!\n");
                return false;
	}
        // store tagged data in memory
        tagged_data_t test_data = {.data = 0x0000BEEF, .tag = 1};
        tag_and_store(ptr, test_data);
        long second_operand = 0xDEAD0000; 
	char second_operand_tag = 2;
        long workplace = 32;
	long expected_result;

        // execute atomic memory operation
	if (strcmp(amo_ins,"or") == 0) {
        	asm volatile (  "amoor.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = test_data.data | second_operand;
	} else if (strcmp(amo_ins,"and") == 0) {
        	asm volatile (  "amoand.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = test_data.data & second_operand;
	} else if (strcmp(amo_ins,"xor") == 0) {
        	asm volatile (  "amoxor.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = test_data.data ^ second_operand;
	} else if (strcmp(amo_ins,"swap") == 0) {
		second_operand = tag_data(second_operand, second_operand_tag);
        	asm volatile (  "wrt a3, %1, %3\n"
				"amoswap.d %0, a3, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr), "r"(second_operand_tag)
				: "a3");
        	expected_result = second_operand;
	} else if (strcmp(amo_ins,"max") == 0) {
        	asm volatile (  "amomax.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = MAX(test_data.data, second_operand);
	} else if (strcmp(amo_ins,"maxu") == 0) {
        	asm volatile (  "amomaxu.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = (long) MAX((unsigned long) test_data.data,
					     (unsigned long) second_operand);
	} else if (strcmp(amo_ins,"min") == 0) {
        	asm volatile (  "amomin.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = MIN(test_data.data, second_operand);
	} else if (strcmp(amo_ins,"minu") == 0) {
        	asm volatile (  "amominu.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = (long) MIN((unsigned long) test_data.data,
					     (unsigned long) second_operand);
	} else if (strcmp(amo_ins,"add") == 0) {
        	asm volatile (  "amoadd.d %0, %1, ( %2 )"
                	        : "=r"(workplace)
                        	: "r" (second_operand), "r"(ptr));
        	expected_result = test_data.data + second_operand;
	} else {
		printf("AMO test: unrecognized operation: s\n", amo_ins);
		return false;
	}
        tagged_data_t result = load_tagged_data(ptr);
	
	// check result
        if((result.data != expected_result)) {
                printf("AMO%s test: The result does not match the expected result!\ndata: 0x%x, tag: 0x%x\n", amo_ins, test_data.data, test_data.tag);
                return false;
        }
        char workplace_tag = get_tag(workplace);
	if(workplace_tag != test_data.tag) {
		printf("AMO%s test: The workplace tag does not match the initial tag!\ndata: 0x%x, tag: 0x%x\n", amo_ins, test_data.data, test_data.tag);
		return false;
	}
	bool isSwap = strcmp(amo_ins, "swap") == 0;
	if( (!isSwap && (result.tag != 0)) ||
	    ( isSwap && (result.tag != second_operand_tag)) ) {
		printf("AMO%s test: The resulting memory tag does not match the exexpected value!\ndata: 0x%x, tag: 0x%x\n", amo_ins, test_data.data, test_data.tag);
		return false;
	}
	return true;
}

int main(int argc, char** argv) {
	bool passed = true;
	passed &= check_amo("or");
	passed &= check_amo("and");
	passed &= check_amo("xor");
	passed &= check_amo("swap");
	passed &= check_amo("max");
	passed &= check_amo("maxu");
	passed &= check_amo("min");
	passed &= check_amo("minu");
	passed &= check_amo("add");

	if(passed) {
		printf("AMO test: passed all tests!\n");
	} else {
		printf("AMO test: failed!\n");
	}
}
