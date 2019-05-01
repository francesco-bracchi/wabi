#include <unistd.h>

#define WABI_READER_BUFFER_SIZE 4096;
#define WABI_READ_BINARY_SIZE 128;


static inline void
wabi_whitespace(int fd,
                char* cur)
{
  while(cur == ' ' || cur == '\n' || cur == '\t') {
    *cur = getc(fd);
  }
}


static inline void
wabi_next(int fd,
          char* cur)
{
  *cur = getch(fd);
}


static wabi_val
wabi_read_rec(wabi_vm vm,
              int fd,
              char* cur);


static wabi_val
wabi_read_list(wabi_vm vm,
               int fd,
               char* cur)
{
  wabi_val car, cdr, res;
  wabi_whitespace(fd, cur);
  car = wabi_read_rec(vm, fd, cur);
  if(vm->errno) return NULL;
  wabi_whitespace(fd, cur);

  switch(*cur) {
  case ')':
    wabi_next(fd, cur);
    cdr = wabi_nil(vm);
    break;
  case '.':
    wabi_next(fd, cur);
    cdr = wabi_read_rec(vm, fd, cur);
    break;
  default:
    cdr = wabi_read_list(vm, fd, cur);
    break;
  }
  if(vm->errno) return NULL;
  return wabi_cons(vm, car, cdr);
}


static wabi_val
wabi_read_binary(wabi_vm vm,
                 int fd,
                 char* cur)
{
  char* blob, limit;

  wabi_binary_leaf leaf;
  leaf = (wabi_binary_leaf) wabi_binary_new(vm, WABI_READ_BINARY_SIZE);
  if(vm->errno) return NULL;

  blob = (char*) leaf->data_ptr;
  limit = blob + WABI_READ_BINARY_SIZE;
  while(blob < limit) {
    wabi_next(fd, cur);
    switch(*cur) {
    case '"':
      wabi_next(fd, cur);
      return wabi_binary_sub_leaf(vm, leaf, 0, blob - (char*) leaf->data_ptr);
      /* case '\\': */
      /*   escape; */
    default:
      *blob = *cur;
      blob++;
    }
  }
  wabi_val more = wabi_read_binary(vm, fd, cur);
  if(vm->errno) return NULL;
  return wabi_binary_concat_raw(vm, leaf, more);
}


static wabi_val
wabi_read_rec(wabi_vm vm,
              int fd,
              char* cur)
{
  wabi_whitespace(fd, cur);
  switch(*cur) {
  case '(':
    wabi_next(fd, cur);
    return wabi_read_list(vm, fd, cur);
  case '"':
    wabi_next(fd, cur);
    return wabi_read_binary(vm, fd, cur);
  /* case '{': */
  /*   return wabi_read_map(vm, fd); */
  /* case '[': */
  /*   return wabi_read_sequence(vm, fd); */
  default:
    if(*cur >= '0' && *cur <= '9') {
      return wabi_read_number(vm, fd, cur);
    }
    else {
      return wabi_read_symbol(vm, fd, cur);
    }
  }
}

wabi_val
wabi_read_raw(wabi_vm vm,
              int fd)
{
  cur = getc(fd);
  return wabi_read_rec(vm, fd, &c);
}



/* typedef struct wabi_reader_struct { */
/*   int fd; */
/*   char* buffer; */
/*   char* top; */
/*   wabi_pair stack; */
/*   wabi_val result; */
/* } wabi_reader_t; */


/* typedef wabi_reader_t* wabi_reader; */

/* wabi_reader */
/* wabi_reader_init (wabi_vm vm, */
/*                   wabi_reader reader, */
/*                   int fd) */
/* { */
/*   reader->fd= fd; */
/*   reader->buffer = malloc(WABI_READER_BUFFER_SIZE); */
/*   reader->top = reader->buffer; */
/*   stack = wabi_null(vm); */
/*   if(vm->errno) return NULL; */
/* } */

/* wabi_val */
/* wabi_read(wabi_vm vm, */
/*           wabi_reader reader) */
/* { */
/*   wabi_val expr; */
/*   int bytes; */
/*   for(;;) { */
/*     expr = wabi_reader_process(vm, reader); */
/*     if(expr) return expr; */
/*     bytes = read(fd, reader->buffer, WABI_READER_BUFFER_SIZE); */
/*     if(! bytes) { */
/*       vm->errno = WABI_ERROR_EOF_READ; */
/*       return NULL; */
/*     } */
/*     reader->top = reader->buffer + bytes; */
/*   } */
/*   return expr; */
/* } */


/* wabi_reader_process(wabi_vm vm, */
/*                     wabi_reader reader) */
/* { */
/*   wabi_val expr; */
/*   char* cur, top; */
/*   cur = reader->buffer; */
/*   top = reader->top; */
/*   while(cur < top) { */
/*     switch(&cur) { */
/*       ' ' -> */
/*         continue; */
/*       '(' -> */
/*         return read_list(vm, reader, cur, top); */

/*     } */
/*   } */
/* } */
