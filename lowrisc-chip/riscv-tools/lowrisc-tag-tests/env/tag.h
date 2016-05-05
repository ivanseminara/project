// See LICENSE for license details.

#ifndef LOWRISC_TAG_H
#define LOWRISC_TAG_H

#define TAG_WIDTH 4

inline int load_tag(void *addr) {
  int rv = 32;
  asm volatile ("ltag %0, 0(%1)"
                :"=r"(rv)
                :"r"(addr)
                );
  return rv;
}

inline void store_tag(void *addr, int tag) {
  asm volatile ("stag %0, 0(%1)"
                :
                :"r"(tag), "r"(addr)
                );
}

inline int tag_check_and_load(void *addr) {
	int rv = 32;
	asm volatile ("ldct %0, 0(%1)" : "=r"(rv) : "r"(addr) );
	return rv;
}

inline void tag_check_and_store(void *addr, long value) {
	asm volatile ("sdct %0, 0(%1)" : : "r"(value), "r"(addr) );
}

/**
 * Update the ld_tag csr to value and return the old value.
 */
inline int read_write_load_csr(int value) {
	int rv = 32, index = 4;
	//int test = csr_read(4);
	asm volatile ("csrrw %0, 4, %1" : "=r"(rv) : "r"(value) );
	return rv;
}
/**
 * Update the sd_tag csr to value and return the old value.
 */
inline int read_write_store_csr(int value) {
	int rv = 32;
	asm volatile ("csrrw %0, 5, %1" : "=r"(rv) : "r"(value) );
	return rv;
}

/**
 * helper functions for tagged register tests
 */
typedef struct {
long data;
char tag;
} tagged_data_t;

inline void tag_and_store(void *addr, tagged_data_t tdata) {
	asm volatile (	"wrt %0, %1, %2\n"
			"sd %0, 0(%3)\n"
			"wrt %0, %1, zero\n"
			: "+r"(tdata.data)
			: "r" (tdata.data), "r"(tdata.tag), "r"(addr) );
	// also zero out the tag after you wrote it to prevent a
	// stale tag messing with your stuff!
}

inline tagged_data_t load_tagged_data(void *addr) {
	long rv = 32;
	char tag = 32;
	asm volatile ("ld %0, 0(%1)" : "=r"(rv) : "r"(addr) );
	asm volatile ("rdt %0, %1" : "=r"(tag) : "r"(rv) );
	return {.data = rv, .tag = tag};
}
inline void tag_and_store_check(void *addr, tagged_data_t tdata) {
	asm volatile ("wrt  %0, %1, %2" : "=r"(tdata.data) : "r"(tdata.data), "r"(tdata.tag) );
	asm volatile ("sdct %0, 0(%1)" : : "r"(tdata.data), "r"(addr) );
}

inline tagged_data_t load_tagged_data_check(void *addr) {
	long rv = 32;
	char tag = 32;
	asm volatile ("ldct %0, 0(%1)" : "=r"(rv) : "r"(addr) );
	asm volatile ("rdt %0, %1" : "=r"(tag) : "r"(rv) );
	return {.data = rv, .tag = tag};
}

inline long tag_data(long data, char tag) {
	long rv = 32;
	asm volatile ("wrt %0, %1, %2" : "=r"(rv) : "r"(data), "r"(tag) );
	return rv;
}
inline char get_tag(long data) {
	char result = 32;
	asm volatile ("rdt %0, %1" : "=r"(result) : "r"(data) );
	return result;
}
#endif
