#include<stdlib.h>
#include<stdio.h>
#include<env/full_empty.h>


bool load_if_full_test(bool full_location) {
	
	char mem_tag = full_location ? TAG_FULL : TAG_EMPTY;
	long *location = (long*) malloc(sizeof(long));
	if (location == NULL) {
		fprintf(stderr, "Error - could not allocate enough memory!");
		return false;
	}
	tag_and_store(location, {.data = 0xBEEF, .tag = mem_tag});

	tagged_data_t initial_mem = load_tagged_data(location);
	printf("initial memory: data: %lx, tag: %d\n", initial_mem.data, initial_mem.tag);

	long result;
	bool success = load_fe(location, &result);

	char new_mem_tag = load_tag(location);

	printf("full memory: %d, load successful: %d, data: %lx, new mem tag: %d\n",
		full_location, success, result, new_mem_tag);

	return (full_location && success) || (!full_location && !success);
}

bool store_if_empty_test(bool empty_location) {
	long data = 0xBEEF;
	char mem_tag = empty_location ? TAG_EMPTY : TAG_FULL;
	long *location = (long *) malloc(sizeof(long));
	if (location == NULL) {
		fprintf(stderr, "Error - could not allocate enough memory!");
		return false;
	}
	store_tag(location, mem_tag);

	tagged_data_t initial_mem = load_tagged_data(location);
	printf("initial memory: data: %lx, tag: %d\n", initial_mem.data, initial_mem.tag);
	
	bool success = store_ef(location, data);

	tagged_data_t result = load_tagged_data(location);

	printf("empty memory: %d, store successful: %d, new mem value: %lx, new mem tag: %d\n",
		empty_location, success, result.data, result.tag);

	return (empty_location && success) || (!empty_location && !success);
	
}

int main(int argc, char **argv) {
	bool success = true;
	success = success && load_if_full_test(true);
	success = success && load_if_full_test(false);

	success = success && store_if_empty_test(true);
	success = success && store_if_empty_test(false);

	if(success)
		printf("Passed all tests!\n");
	else
		printf("Failed tests!\n");
}
