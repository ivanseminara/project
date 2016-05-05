/**
 * Description: This contains tests to test the tag propagation between
 * memory and registers on various 64-bit wide memory accesses
 */
#include"env/tag.h"
#include<stdio.h>
#include<stdlib.h>


bool check_load_and_store_with_tag_check(bool check_on_load_and_store) {

	tagged_data_t test_data = {.data = 111, .tag = 1};
	void *ptr = malloc(sizeof(long));
	if(ptr == NULL) {
		fprintf(stderr, "Error - could not initialize enough memory!\n");
		return false;
	}
	const char* tag_check = check_on_load_and_store ? "with tag checks" : "without tag checks";
	printf("Test: Tag a register, store it, load it (%s), \
compare data and tag to initial value\n", tag_check);
	
	tagged_data_t result_data;
	if(check_on_load_and_store) {	// use the instructions that also check the tag
		tag_and_store_check(ptr, test_data);
		result_data = load_tagged_data_check(ptr);
	} else { // use a normal store/load that preserves the tag but won't check it
		tag_and_store(ptr, test_data);
		result_data = load_tagged_data(ptr);
	}

	if(test_data.data == result_data.data)
		printf("Data has been preserved during store and load\n");
	else {
		printf("Data could not be preserved during store and load\n");
		return false;
	}
	if(test_data.tag == result_data.tag)
		printf("Tag has been preserved during store and load\n");
	else {
		printf("Tag could not be preserved during store and load\n");
		return false;
	}
	return true;
}

bool check_reset_on_ALU() {
	long test_long = 0xDEADBEEE;
	char tag = 1, new_tag;
	printf("Tag a value (in a register), increment and inspect the tag\n");
	// you need to use inline assembly to make sure that the ALU result is written
	// into the register we previously tagged!
	asm volatile (	"wrt a5, %1, %2\n"
			"addi a5, a5, 1\n"
			"rdt %0, a5\n"
			: "=r"(new_tag)
			: "r"(test_long), "r"(tag) 
			: "a5");
	if(new_tag == 0) 
		printf("The tag of 0x%x has been successfully reset to 0 on an ALU operation\n", test_long);
	else {
		printf("The tag of 0x%x is 0x%02x after the ALU operation\n", test_long, new_tag);
		return false;
	}
	return true;
}

bool check_lr_sc() {
	// TODO
	return true;
}

int main(int argc, char **argv) {

	bool success = true;
	success = success && check_load_and_store_with_tag_check(true);
	success = success && check_load_and_store_with_tag_check(false);
	success = success && check_reset_on_ALU();

	if(success) printf("All tests passed\n");
	else printf("Tests failed\n");

}
