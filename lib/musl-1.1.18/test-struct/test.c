#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
void main() {
  struct stat st;
  printf("st_dev %ld\n", (unsigned long)&(st.st_dev) - (unsigned long)&st);
  printf("st_ino %ld\n", (unsigned long)&(st.st_ino) - (unsigned long)&st);
  printf("st_nlink %ld\n", (unsigned long)&(st.st_nlink) - (unsigned long)&st);
  printf("st_mode %ld\n", (unsigned long)&(st.st_mode) - (unsigned long)&st);
  printf("st_uid %ld\n", (unsigned long)&(st.st_uid) - (unsigned long)&st);
  printf("st_gid %ld\n", (unsigned long)&(st.st_gid) - (unsigned long)&st);
  printf("__pad0 %ld\n", (unsigned long)&(st.__pad0) - (unsigned long)&st);
  printf("st_rdev %ld\n", (unsigned long)&(st.st_rdev) - (unsigned long)&st);
  printf("st_size %ld\n", (unsigned long)&(st.st_size) - (unsigned long)&st);
  printf("st_blksize %ld\n", (unsigned long)&(st.st_blksize) - (unsigned long)&st);
  printf("st_blocks %ld\n", (unsigned long)&(st.st_blocks) - (unsigned long)&st);
  printf("st_atim %ld\n", (unsigned long)&(st.st_atim) - (unsigned long)&st);
  printf("st_mtim %ld\n", (unsigned long)&(st.st_mtim) - (unsigned long)&st);
  printf("st_ctim %ld\n", (unsigned long)&(st.st_ctim) - (unsigned long)&st);
  printf("__glibc_reserved %ld\n", (unsigned long)&(st.__glibc_reserved) - (unsigned long)&st);
}
