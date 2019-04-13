1. change the pair order?
   encoding the tail part in the first word, since it's always a reference,
   the car part has a whole word to inline simple values. Is it worth the
   effort?

1. have the very same location for nil/true/false

1. is worth using hamt as a write on copy mean to a file? (like RocksDB)

1. Use a full index table for submaps bigger than, say 32 elements (to
   be evaluated)
