#include <string.h>
#include <_ansi.h>
#include <stddef.h>
#include <limits.h>

/* FIXME should be defined by gcc if applicable */
#ifdef __riscv64
# define TAGGED_MEMORY
#endif

#ifdef TAGGED_MEMORY
#include "sys/platform/tag.h"
#endif

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED2(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* Nonzero if either X, Y or Z is not aligned on a "long" boundary.  */
#define UNALIGNED3(X, Y, Z) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)) | \
  ((long)Z & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

#ifdef TAGGED_MEMORY
#define MEMMOVE_NAME __riscv_memmove_no_tags
#else
#define MEMMOVE_NAME memmove
#endif
void* MEMMOVE_NAME(void *dst_void, const void* src_void, size_t length) {

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

  char *dst = dst_void;
  _CONST char *src = src_void;

  if (src < dst && dst < src + length)
    {
      /* Have to copy backwards */
      src += length;
      dst += length;
      while (length--)
	{
	  *--dst = *--src;
	}
    }
  else
    {
      while (length--)
	{
	  *dst++ = *src++;
	}
    }

  return dst_void;
#else
  char *dst = dst_void;
  _CONST char *src = src_void;
  long *aligned_dst;
  _CONST long *aligned_src;
  int   len =  length;

  if (src < dst && dst < src + len)
    {
      /* Destructive overlap...have to copy backwards */
      src += len;
      dst += len;
      while (len--)
	{
	  *--dst = *--src;
	}
    }
  else
    {
      /* Use optimizing algorithm for a non-destructive copy to closely 
         match memcpy. If the size is small or either SRC or DST is unaligned,
         then punt into the byte copy loop.  This should be rare.  */
      if (!TOO_SMALL(len) && !UNALIGNED2 (src, dst))
        {
          aligned_dst = (long*)dst;
          aligned_src = (long*)src;

          /* Copy 4X long words at a time if possible.  */
          while (len >= BIGBLOCKSIZE)
            {
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              len -= BIGBLOCKSIZE;
            }

          /* Copy one long word at a time if possible.  */
          while (len >= LITTLEBLOCKSIZE)
            {
              *aligned_dst++ = *aligned_src++;
              len -= LITTLEBLOCKSIZE;
            }

          /* Pick up any residual with a byte copier.  */
          dst = (char*)aligned_dst;
          src = (char*)aligned_src;
        }

      while (len--)
        {
          *dst++ = *src++;
        }
    }

  return dst_void;
#endif /* not PREFER_SIZE_OVER_SPEED */
}

#ifdef TAGGED_MEMORY

/* All arguments must be long-aligned, and the length is in long's. */
unsigned long* __riscv_memmove_tagged_longs(unsigned long *dst, 
  const unsigned long* src, size_t length) {

#define TAG_COPY_BACKWARD(d, s, length) do { \
  d += length; \
  s += length; \
  s--; d--; \
  for(;length--;s--,d--) { \
    __riscv_tagged_data_t v = __riscv_load_tagged_data(s); \
    __riscv_store_tagged_data(d, v); \
  } \
} while(0);

#define TAG_COPY_FORWARD(d, s, length) do { \
  for(;length--;s++,d++) { \
    __riscv_tagged_data_t v = __riscv_load_tagged_data(s); \
    __riscv_store_tagged_data(d, v); \
  } \
} while(0)

  unsigned long * const ret = dst;

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

  if (src < dst && dst < src + length)
    {
      TAG_COPY_BACKWARD(dst, src, length);
      return ret;
    }
  else
    {
      TAG_COPY_FORWARD(dst, src, length);
      return ret;
    }
#else
  if (src < dst && dst < src + length)
    {
      TAG_COPY_BACKWARD(dst, src, length);
      return ret;
    }
  else
    {
      /* Use optimizing algorithm for a non-destructive copy to closely 
         match memcpy. */

      /* Copy 4X long words at a time if possible.  */
      while (length >= 4)
        {
#define COPY(aligned_dst, aligned_src) do { \
  __riscv_tagged_data_t v = __riscv_load_tagged_data(aligned_src++); \
  __riscv_store_tagged_data(aligned_dst++, v); \
} while(0);

          COPY(dst, src);
          COPY(dst, src);
          COPY(dst, src);
          COPY(dst, src);
          length -= 4;
        }

      /* Copy one long word at a time if possible.  */
      while (length >= 1)
        {
          COPY(dst, src)
          length--;
        }
    }

  return ret;
#endif /* not PREFER_SIZE_OVER_SPEED */
}

void* memmove(void *dst, const void* src, size_t length) {
  if(UNALIGNED3(dst, src, length))
    return __riscv_memmove_no_tags(dst, src, length);
  else
    return __riscv_memmove_tagged_longs((unsigned long*)dst, 
      (const unsigned long*)src, length/sizeof(unsigned long));
}

#endif /* TAGGED_MEMORY */
