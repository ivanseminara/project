#include"env/tag.h"
#include<stdio.h>
#include<stdlib.h>
#include<string>

/**
 */
bool check_lr(long *ptr) {
	
	// 1. store tagged data 
	tagged_data_t test_data = {.data = 0xBEEF, .tag = 1};
	tag_and_store(ptr, test_data);
	long result_tag = 32, result_data;
	// 2. link and register
	asm volatile (	"lr.d %0, (%1)"
			: "=r" (result_data) : "r"(ptr) );
	asm volatile (	"rdt  %0, %1"
			: "=r" (result_tag) : "r"(result_data) );
	// 3. check data and tag
	if(result_tag != test_data.tag) {
		printf("LL test: the tag was not preserved\n");
		return false;
	}
	if(result_data != test_data.data) {
		printf("LL test: the data was not preserved\n");
		return false;
	}

	return true;
}

bool check_sc(long *ptr) {
	// 1. store tagged data conditionally
	long initial_data = 0xBEEF;
	char initial_tag = 2;
	//long tagged_register = tag_data(initial_data, initial_tag);
	long tagged_register = 32;
	asm volatile ("wrt %0, %1, %2" : "=r"(tagged_register) : "r"(initial_data), "r"(initial_tag));
	int fail = 32;
	asm volatile ("sc.d %0, %1, (%2)" 
			: "=r"(fail) : "r"(tagged_register), "r"(ptr) );

	// 2. load stored data
	tagged_data_t result = load_tagged_data(ptr);
	// 3. check data and tag
	if(!fail && (result.data != initial_data)) {
		printf("SC test: the data was not preserved\n");
		return false;
	}
	if(result.tag != initial_tag) {
		printf("SC test: the tag was not preserved\n");
		return false;
	}
	return true;
}
	

int main(int argc, char** argv) {
	bool passed = true;

	long *ptr = (long *) malloc(sizeof(long));
	if( ptr == NULL) {
		fprintf(stderr, "Error: could not allocate enough memory!\n");
		return false;
	}

	passed &= check_lr(ptr);
	passed &= check_sc(ptr);

	if(passed) {
		printf("LRSC test: passed all tests!\n");
	} else {
		printf("LRSC test: failed!\n");
	}
}
