#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<sched.h>
#include<env/full_empty.h>
#include<errno.h>

int workload = 1;

inline long load_fun(void *addr) {
	long result;
	#ifdef SCHEDUL
		bool retry = !load_fe(addr, &result);
		while(retry) {
			sched_yield();
			retry = !load_fe(addr, &result);
		}
	#else
		while(!load_fe(addr, &result));
	#endif
	return result;
}

inline void store_fun(void *addr, long value) {
	#ifdef SCHEDUL
		bool retry = !store_ef(addr, value);
		while (retry) {
			sched_yield();
			retry = !store_ef(addr, value);
		}
	#else
		while(!store_ef(addr, value));
	#endif
}

/**
 * Load a long from the memory address slot, double the long
 * and store it at the location succeeding slot
 */
void *double_value (void *slot) {
	int i;
	for (i = 0; i < workload; i++) {
		long value = load_fun(slot);
		long *next_slot = ((long *) slot) + 1;

		// yield this thread to execute another one if you cannot load the value
		store_fun(next_slot, value*2);
		//printf("workload %d, slot 1: %lx, slot 2: %lx\n", i, slot, next_slot);
	}
}

void *init_pipeline(void *first_slot) {
	int i;
	for (i = 1; i<= workload; i++) {
		store_fun(first_slot, i);
		//printf("First pipeline slot at %lx initialized to 0x%x\n", first_slot, i);
	}
}

bool double_pipeline_test(int pipeline_length, int p_workload) {
	workload = p_workload;
	
	long *pipeline = (long *) malloc(sizeof(long) * pipeline_length);
	if(pipeline == NULL) {
		fprintf(stderr, "Error - could not allocate enough memory!");
		return false;
	}
	// create pipeline_length -1 threads, each will read the 
	// value from a slot, multipy it by two and write it into
	// the next slot of the pipeline
	int i, ret;
	pthread_t thread;
	// use a thread to initialize the first pipeline slot multiple times
	ret = pthread_create(&thread, NULL, init_pipeline, (void *)pipeline);
		if(ret) {
			fprintf(stderr, "Error - init thread could not be created, pthread_create() return code: %d\n", ret);
		}

	for(i=0;i < pipeline_length-1; i++) {

		ret = pthread_create(&thread, NULL, double_value,
			 (void* )(pipeline+i));
		if(ret) {
			fprintf(stderr, "Error - %dth thread could not be created, pthread_create() return code: %d\n", i, ret);
			return false;
		}
	}
	// read the results from the last pipeline slot multiple times
	for(i=1; i <= workload; i++) {
		long result = load_fun(pipeline + pipeline_length-1);
		printf("result: 0x%lx at pipeline slot 0x%lx\n", result,
			pipeline + pipeline_length-1);
	}
	return true;
}

int main(int argc, char **argv) {
	int pipeline_length, workload;
	if(argc < 3) {
		workload = 1;
	} else {
		workload = (int) strtol(argv[2], (char **)NULL, 10);
	}
	if(argc < 2) {
		pipeline_length = 3;
	} else {
		pipeline_length = (int) strtol(argv[1], (char **)NULL, 10);
	}
	clock_t start, end;

	start = clock();
	bool success = double_pipeline_test(pipeline_length, workload);
	end = clock();
	clock_t ticks = end-start;
	printf("Executed in %ld clock ticks\n", ticks);
}
