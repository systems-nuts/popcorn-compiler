#!/usr/bin/gdb -x

import sys
import gdb
import gdb.types

print("#include <sys/types.h>")
print("#include <sys/stat.h>")
print("#include <unistd.h>")
print("#include <stdio.h>")
print("void main() {")
print("  struct stat st;")

gdb.execute("file ./st") #" + sys.argv[0])
my_struct = gdb.lookup_type("struct stat") #sys.argv[1])
for key in my_struct.keys():
	print("  printf(\"%s %%d\\n\", (int)((unsigned long)&(st.%s) - (unsigned long)&st));" % (key, key))
	
print("}")
	
gdb.execute("quit")
  
