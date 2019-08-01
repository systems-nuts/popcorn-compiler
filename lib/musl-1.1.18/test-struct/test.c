#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
void main() {
  struct stat st;
  printf("st_dev %d\n", (unsigned long)&(st.st_dev) - (unsigned long)&st);
  printf("st_ino %d\n", (unsigned long)&(st.st_ino) - (unsigned long)&st);
  printf("st_nlink %d\n", (unsigned long)&(st.st_nlink) - (unsigned long)&st);
  printf("st_mode %d\n", (unsigned long)&(st.st_mode) - (unsigned long)&st);
  printf("st_uid %d\n", (unsigned long)&(st.st_uid) - (unsigned long)&st);
  printf("st_gid %d\n", (unsigned long)&(st.st_gid) - (unsigned long)&st);
  printf("__pad0 %d\n", (unsigned long)&(st.__pad0) - (unsigned long)&st);
  printf("st_rdev %d\n", (unsigned long)&(st.st_rdev) - (unsigned long)&st);
  printf("st_size %d\n", (unsigned long)&(st.st_size) - (unsigned long)&st);
  printf("st_blksize %d\n", (unsigned long)&(st.st_blksize) - (unsigned long)&st);
  printf("st_blocks %d\n", (unsigned long)&(st.st_blocks) - (unsigned long)&st);
  printf("st_atim %d\n", (unsigned long)&(st.st_atim) - (unsigned long)&st);
  printf("st_mtim %d\n", (unsigned long)&(st.st_mtim) - (unsigned long)&st);
  printf("st_ctim %d\n", (unsigned long)&(st.st_ctim) - (unsigned long)&st);
  printf("__glibc_reserved %d\n", (unsigned long)&(st.__glibc_reserved) - (unsigned long)&st);
}
