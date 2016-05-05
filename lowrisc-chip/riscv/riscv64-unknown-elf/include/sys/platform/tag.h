/* Platform-specific functions for tagged memory on LowRISC.
 *
 * The compiler should define __TAGGED_MEMORY__ and __lowrisc__ if these are 
 * available. At present only RISCV-LLVM does this, not GCC.
 *
 * You may want to use tags for your own purposes, and might want to use the 
 * memcpy/memmove variants, although your compiler should choose the right one 
 * in many cases, and the default behaviour should generally be compatible with 
 * existing code while not providing the maximum possible protection. */

#if defined(__riscv64) || defined(TAGGED_MEMORY)

#ifndef LOWRISC_TAG_H
#define LOWRISC_TAG_H

#include <assert.h>
#include <stddef.h>

#define TAG_WIDTH 4

/* Standard tag values */

/* No effect. */
#define __RISCV_TAG_NONE 0
/* Fault on write. */
#define __RISCV_TAG_READ_ONLY 1
/* Fault on read. Cleared on "normal" write, so "write then read". */
#define __RISCV_TAG_WRITE_ONLY 2
/* Fault on read or write. */
#define __RISCV_TAG_INVALID 3
/* Function pointer */
#define __RISCV_TAG_CLEAN_FPTR 4
/* Pointer to function pointer (e.g. vtable) */
#define __RISCV_TAG_CLEAN_PFPTR 5
/* Pointer to structure including function pointer etc */
#define __RISCV_TAG_CLEAN_SENSITIVE 6
/* Pointer to structure including void* */
#define __RISCV_TAG_CLEAN_SENSITIVE_VOID 7
/* void* (or char*) */
#define __RISCV_TAG_CLEAN_VOIDPTR 8
/* Any pointer */
#define __RISCV_TAG_CLEAN_POINTER 9

/* Load tag, store tag. May be used in very low level code e.g. memcpy, so 
 * important for it to be inline. */

#ifndef INLINE
# if __GNUC__ && !defined(__GNUC_STDC_INLINE__)
#  define INLINE extern inline __attribute__((always_inline))
# elif __GNUC__ || defined(__clang__)
#  define INLINE inline __attribute__((always_inline))
# else
#  define INLINE inline
# endif
#endif

INLINE unsigned char __riscv_load_tag(const void *addr) {
  unsigned int rv = 32;
  asm volatile ("ltag %0, 0(%1)"
                :"=r"(rv)
                :"r"(addr)
                );
  return (unsigned char) rv;
}

INLINE void __riscv_store_tag(void *addr, unsigned char tag) {
  unsigned int t = tag;
  asm volatile ("stag %0, 0(%1)"
                :
                :"r"(t), "r"(addr)
                );
}

/* Atomic access to tagged values. Thanks Lucas. */

typedef struct {
  unsigned long data;
  char tag;
} __riscv_tagged_data_t;

INLINE void __riscv_store_tagged_data(void *addr, __riscv_tagged_data_t tdata) {
	asm volatile (	"wrt %0, %1, %2\n"
			"sdct %0, 0(%3)\n"
			"wrt %0, %1, zero\n"
			: "+r"(tdata.data)
			: "r" (tdata.data), "r"(tdata.tag), "r"(addr) );
	// also zero out the tag after you wrote it to prevent a
	// stale tag messing with your stuff!
}

INLINE __riscv_tagged_data_t __riscv_load_tagged_data(const void *addr) {
	__riscv_tagged_data_t tdata;
	tdata.data = 32;
	tdata.tag = 32;
	asm volatile ("ldct %0, 0(%1)" : "=r"(tdata.data) : "r"(addr) );
	asm volatile ("rdt %0, %1" : "=r"(tdata.tag) : "r"(tdata.data) );
	return tdata;
}

/* Variants of memcpy and memmove with and without copying tags.
 *
 * SECURITY: You should only copy tags when there is a good reason to do so, as
 * it may bypass tag-based protection (e.g. tagging of code pointers). You 
 * should copy tags if copying structures including function pointers, but not 
 * when copying strings, floats etc, unless adding your own tags.
 *
 * COMPATIBILITY: The default memcpy and memmove will copy tags if all three 
 * arguments (including the length) are long-aligned. Hopefully compilers will
 * do type-based analysis to improve performance and reduce the attack surface. 
 */

/* Copy length bytes from src to dst. Do not copy tags. aa and bb must not 
 * overlap. Identical to a standard memcpy() without tagged memory support. 
 * Should be used for strings, floats etc. */
void* __riscv_memcpy_no_tags(void* dst, const void* src, size_t length);

/* Copy length bytes from src to dst. Do not copy tags. aa and bb may overlap.
 * Identical to a standard memmove() without tagged memory support.
 * Should be used for strings, floats etc where overlap is possible. */
void* __riscv_memmove_no_tags(void *dst, const void* src, size_t length);

/* Copy length long's, including tags, from src to dst. dst and src must not
 * overlap, and must be 64-bit aligned. Use this method for copying structures 
 * that may contain code pointers etc. */
unsigned long *__riscv_memcpy_tagged_longs(unsigned long *dst, 
  const unsigned long* src, size_t length);

/* Copy length long's, including tags, from src to dst. dst and src may 
 * overlap, but must be 64-bit aligned. Use this method for copying structures 
 * that may contain code pointers etc, where dst and src may overlap. */
unsigned long *__riscv_memmove_tagged_longs(unsigned long *dst, 
  const unsigned long* src, size_t length);

/* void* versions. Be careful! */

/* Nonzero if either X, Y or Z is not aligned on a "long" boundary.  */
#define UNALIGNED3(X, Y, Z) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)) | \
  ((long)Z & (sizeof (long) - 1)))

/* Copy length bytes, including tags, from src to dst. dst and src must not
 * overlap, and must be 64-bit aligned, and length must be a multiple of 8. Use 
 * this method for copying structures that may contain code pointers etc. */
INLINE void *__riscv_memcpy_tagged(void *dst, const void *src, size_t length) {
  assert(!UNALIGNED3(dst, src, length));
  return __riscv_memcpy_tagged_longs((unsigned long*)dst, (unsigned long*)src, length / sizeof(long));
}

/* Copy length bytes, including tags, from src to dst. dst and src may 
 * overlap, but must be 64-bit aligned, and length must be a multiple of 8. 
 * Use this method for copying structures that may contain code pointers etc, 
 * where dst and src may overlap. */
INLINE void *__riscv_memmove_tagged(void *dst, const void *src, size_t length) {
  assert(!UNALIGNED3(dst, src, length));
  return __riscv_memmove_tagged_longs((unsigned long*)dst, (unsigned long*)src, length / sizeof(long));
}

#endif /* !LOWRISC_TAG_H */
#endif
