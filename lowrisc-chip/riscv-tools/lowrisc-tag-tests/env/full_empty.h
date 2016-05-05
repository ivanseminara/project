/**
 * Library with helper functions for the full - empty usecase
 */

#define TAG_FULL 1
#define TAG_EMPTY 0

#include"tag.h"

inline long load_when_full(void *addr) {
        long result = tag_check_and_load(addr);
        store_tag(addr, 0);
	return result;
}

inline void store_and_set_full(void *addr, long value) {
	tag_check_and_store(addr, value);
	store_tag(addr, 1);
}

/**
 * Load a value from memory if its tag is set to 'full', and set it
 * to 'empty'.
 * Returns a boolean to indicate whether the load has been successful.
 */
inline bool load_fe(void *addr, long *result) {
	bool success = 32;
	long loaded_value = 32;
	char tag = 32;
	char empty_tag = TAG_EMPTY;
	asm volatile (	"lr.d %0, (%1)\n"
			: "=r"(loaded_value) : "r"(addr) );
	asm volatile (	"rdt %0, %1\n"
			: "=r"(tag) : "r"(loaded_value) );
	if(tag!= TAG_FULL)
		return false;

	asm volatile ( 	"wrt t1, %1, %2\n"
			"sc.d %0, t1, (%3)\n"
			: "=r"(success)
			: "r"(loaded_value), "r"(empty_tag), "r"(addr) 
			: "t1" );
	if(success == 0) {
		*result = loaded_value;
		return true;
	} else
		return false;
}


/**
 * Store the value at the given address if its tag is set to 'empty', and 
 * set the tag to 'full'.
 * Returns a boolean indicating whether the store has been successful.
 */
inline bool store_ef(void* addr, long value) {
	char tag = 32;
	char full_tag = TAG_FULL;
	bool success;
	asm volatile (	"lr.d t1, (%1)\n"
			"rdt %0, t1\n"
			: "=r"(tag) : "r"(addr) : "t1" );
	if(tag != TAG_EMPTY)
		return false;
	asm volatile (	"wrt t1, %1, %2\n"
			"sc.d %0, t1, (%3)\n"
			: "=r"(success)
			: "r"(value), "r"(full_tag), "r"(addr)
			: "t1" );
	return (success == 0);
}

inline void setup_exceptions() {
	read_write_load_csr(1);		// trap on tag 00
					// atm trap means busy lock
	read_write_store_csr(0); 	// don't trap for now
}
