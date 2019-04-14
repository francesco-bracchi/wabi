1. change the pair order?
   encoding the tail part in the first word, since it's always a reference,
   the car part has a whole word to inline simple values. Is it worth the
   effort?

1. have the very same location for nil/true/false

1. is worth using hamt as a write on copy mean to a file? (like RocksDB)

1. Use a full index table for submaps bigger than, say 32 elements (to
   be evaluated)

1. cursors for maps (and eventually sequences)

1. split the collection of symbol table in atomic steps.

1. big hamt maps (i.e. table filled more than x%) should have always
   tables with 64 entries, nulled for not present elements,thus skipping
   the popcnt phase (as in clojure)

1. small hamt maps (i.e. less than branching factor: 64) should be
   represented as array maps, scanned linearly thus skipping hash, and
   popcnt phase.

1. Rehash for HAMT

1. bug for the for loop in the wabi_main_c file if the limit for the j
   variable is 109111;
