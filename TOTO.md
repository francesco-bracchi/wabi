1. change naming conventions:
   wabi_<whatever> -> wabi_<whatever>_wrp -> wrapper to be used in eval context
   wabi_<whatever>_raw -> wabi_<whatever> -> actual c implementation


1. change the pair order?
   encoding the tail part in the first word, since it's always a reference,
   the car part has a whole word to inline simple values. Is it worth the
   effort?

1. have the very same location for nil/true/false

1. rewrite hamt module, wrt the union of wabi_hamt_kw and wabi_hamt_map
