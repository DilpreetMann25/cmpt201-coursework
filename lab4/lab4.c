// lab4.c
// AB-4: Heap Size with sbrk()

#define _DEFAULT_SOURCE
#include <errno.h>    // errno
#include <inttypes.h> // PRIu64
#include <stddef.h>   // size_t
#include <stdint.h>   // uint64_t, uintptr_t
#include <stdio.h>    // snprintf
#include <stdlib.h>   // exit
#include <string.h>   // memset
#include <unistd.h>   // sbrk, write

// ---- printing helpers (use write, avoid printf on stdout) ----
#define BUF_SIZE 256

static void handle_error(const char *what) {
  char buf[BUF_SIZE];
  int n = snprintf(buf, sizeof(buf), "ERROR: %s (errno=%d)\n", what, errno);
  if (n > 0)
    write(STDERR_FILENO, buf, (size_t)n);
  _exit(1);
}

// As provided in the lab (slightly defensive if data is NULL)
static void print_out(const char *format, const void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len;

  if (!data) {
    len = snprintf(buf, BUF_SIZE, format, (void *)NULL);
  } else if (data_size == sizeof(uint64_t)) {
    len = snprintf(buf, BUF_SIZE, format, *(const uint64_t *)data);
  } else {
    len = snprintf(buf, BUF_SIZE, format, *(void *const *)data);
  }

  if (len < 0)
    handle_error("snprintf");
  if (write(STDOUT_FILENO, buf, (size_t)len) < 0)
    handle_error("write");
}

// Print one byte (as decimal) on its own line, using uint64_t path.
static void print_byte(uint8_t b) {
  uint64_t v = (uint64_t)b;
  print_out("%" PRIu64 "\n", &v, sizeof(v));
}

// ---- Block layout ----
struct header {
  uint64_t size;       // total bytes in this block, INCLUDING header
  struct header *next; // singly linked list pointer
};

enum { TOTAL_HEAP_INC = 256, BLOCK_BYTES = 128 };

int main(void) {
  // 1) Grow heap by 256 bytes
  void *old_brk = sbrk(0);
  if (old_brk == (void *)-1)
    handle_error("sbrk(0)");
  if (sbrk(TOTAL_HEAP_INC) == (void *)-1)
    handle_error("sbrk(+256)");

  // Base of our newly obtained region
  unsigned char *base = (unsigned char *)old_brk;

  // 2) Two equal-sized blocks of 128 bytes each (including header)
  struct header *first = (struct header *)(base);
  struct header *second = (struct header *)(base + BLOCK_BYTES);

  // Sanity: block data sizes
  size_t header_sz = sizeof(struct header);
  size_t data_sz = (size_t)BLOCK_BYTES - header_sz;

  // Initialize headers
  first->size = BLOCK_BYTES;
  first->next = NULL;
  second->size = BLOCK_BYTES;
  second->next = first;

  // Initialize data (bytes after header)
  unsigned char *first_data = (unsigned char *)(first + 1);
  unsigned char *second_data = (unsigned char *)(second + 1);

  memset(first_data, 0, data_sz);
  memset(second_data, 1, data_sz);

  // 3) Printing (addresses, header fields, then all bytes)
  // Addresses of each block (starting addresses = header addresses)
  print_out("first block:       %p\n", (void *)&first, sizeof(&first));
  print_out("second block:      %p\n", (void *)&second, sizeof(&second));

  // Header values
  print_out("first block size:  %" PRIu64 "\n", &first->size,
            sizeof(first->size));
  print_out("first block next:  %p\n", &first->next, sizeof(first->next));
  print_out("second block size: %" PRIu64 "\n", &second->size,
            sizeof(second->size));
  print_out("second block next: %p\n", &second->next, sizeof(second->next));

  // Contents of block data (exclude headers)
  for (size_t i = 0; i < data_sz; ++i)
    print_byte(first_data[i]);
  for (size_t i = 0; i < data_sz; ++i)
    print_byte(second_data[i]);

  return 0;
}
