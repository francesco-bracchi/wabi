#ifndef wabi_secd_h

#define wabi_secd_h


typedef struct wabi_secd_frame_struct {
  wabi_val stack;
  wabi_val environment;
  wabi_val control;
  wabi_val previous;
} wabi_secd_frame_t;

typedef wabi_secd_frame_t* wabi_secd_frame;

typedef struct wabi_secd_struct {
  // todo leverage register keyword
  wabi_val stack;
  wabi_val environment;
  wabi_val control;
  wabi_val dump;
  wabi_val nil;
  wabi_val store;
  unsigned int fuel;
  int errno;
} wabi_secd_t;

typedef wabi_secd_t* wabi_secd;

#define WABI_SECD_STORE_SIZE 1000000

#define WABI_SECD_FRAME_BYTE_SIZE 32
#define WABI_SECD_FRAME_SIZE 4

typedef enum wabi_word_enum {
   wabi_word_push = 0,
   wabi_word_eval = 1,
   wabi_word_join = 4,
   wabi_word_oper = 5,
   wabi_word_ap   = 6,
   wabi_word_dum  = 7,

} wabi_word;

int
wabi_secd_init(wabi_secd vm);

#endif
