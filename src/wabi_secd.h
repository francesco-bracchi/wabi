#ifndef wabi_secd_h

#define wabi_secd_h


typedef struct wabi_secd_frame_struct {
  wabi_word_t stack;
  wabi_word_t environment;
  wabi_word_t control;
  wabi_word_t ret;
} wabi_secd_frame_t;

typedef wabi_secd_frame_t* wabi_secd_frame;

typedef struct wabi_secd_struct {
  // todo leverage register keyword
  wabi_word_t stack;
  wabi_word_t environment;
  wabi_word_t control;
  wabi_word_t ret;

  wabi_word_t dump;
  wabi_word_t vocabulary;
  wabi_word_t nil;
  wabi_store store;
  int errno;
  wabi_word_t fuel;
} wabi_secd_t;

typedef wabi_secd_t* wabi_secd;

#define WABI_SECD_STORE_SIZE 1000000

#define WABI_SECD_FRAME_BYTE_SIZE 32
#define WABI_SECD_FRAME_SIZE 4

int
wabi_secd_init(wabi_secd vm);

#endif
