#!/usr/bin/gdb -x

import sys
import gdb
import gdb.types

gdb.execute("macro define offsetof(t, f) &((t *) 0)->f)")

gdb.execute("file ./st") #" + sys.argv[0])
my_struct = gdb.lookup_type("struct stat") #sys.argv[1])
for key in my_struct.keys():
  gdb.execute("p offsetof(struct stat, %s)" % key)
gdb.execute("quit")
