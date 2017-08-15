#include "basic_fs_mem.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BasicFSMem::BasicFSMem(char *name, unsigned long size) : BasicFS(size){
  int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);

  if(!size) {
    struct stat statbuf;
    fstat(fd, &statbuf);
    size = statbuf.st_size;
  }
  disk_handler = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (disk_handler == MAP_FAILED){
    perror("BasicFSMem");
    exit(-1);
  }
}
BasicFSMem::~BasicFSMem(){
}

int BasicFSMem::read_block(int block, char *buffer){
  memcpy(buffer, disk_handler + (block * BLOCK_SIZE), BLOCK_SIZE);
}

int BasicFSMem::write_block(int block, char *buffer){
  memcpy(disk_handler + (block * BLOCK_SIZE), buffer, BLOCK_SIZE);
}

extern "C" BasicFS *loadBasicFS(char *name, unsigned long size) {
  return new BasicFSMem(name, size);
}
