#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<sched.h>
#include<env/full_empty.h>

/**
 * Load a long from the memory address slot, double the long
 * and store it at the location succeeding slot
 */
void *double_value (void *slot) {
	// busy spin until you can load the value
	long value;
	while(!load_fe(slot, &value));
	long *next_slot = ((long *) slot) + 1;

	// busy spin until you can store the value
	while(!store_ef(next_slot, value * 2));
	printf("slot 1: %lx, slot 2: %lx\n", slot, next_slot);
}

bool double_pipeline_test(int pipeline_length) {
	
	long *pipeline = (long *) malloc(sizeof(long) * pipeline_length);
	if(pipeline == NULL) {
		fprintf(stderr, "Error - could not allocate enough memory!");
		return false;
	}
	
	// initialize the first pipeline slot to 1
	while(!store_ef(pipeline, 1));
	printf("First pipeline slot at %lx initialized to %d\n", pipeline, 1);
	
	// create pipeline_length -1 threads, each will read the 
	// value from a slot, multipy it by two and write it into
	// the next slot of the pipeline
	int i, ret;
	pthread_t thread;
	for(i=0;i < pipeline_length-1; i++) {
		ret = pthread_create(&thread, NULL, double_value,
			 (void* )(pipeline+i));
		if(ret) {
			fprintf(stderr, "Error - %dth thread could not be created, pthread_create() return code: %d\n", i, ret);
			return false;
		}
	}
	long result;
	while(!load_fe(pipeline + pipeline_length-1, &result));
	printf("result: %d at pipeline slot 0x%lx\n", result,
		pipeline + pipeline_length-1);

	return true;
}

int main(int argc, char **argv) {
	int pipeline_length;
	if(argc < 2) {
		pipeline_length = 3;
	} else {
		pipeline_length = (int) strtol(argv[1], (char **)NULL, 10);
	}
	bool success = double_pipeline_test(pipeline_length);
}
