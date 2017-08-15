#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "filesystem.h"

#define BLOCK_SIZE 4096

enum size_values {
  B = 1,
  KB = 1024,
  MB = 1024 * 1024,
  GB = 1024 * 1024 * 1024
};

unsigned long get_size(char *size);
void create_file(unsigned long size, char *name);
void create_mem(unsigned long size, char *name);

int main(int argc, char **argv){
  int flag;
  unsigned long disk_size;
  char *file_name = NULL, *mem_name = NULL;

  while((flag = getopt(argc, argv, "m:f:")) != -1)
    switch(flag) {
      case 'f':
        file_name = (char*)malloc(strlen(optarg));
        strcpy(file_name, optarg);
        break;
      case 'm':
        mem_name = (char*)malloc(strlen(optarg));
        strcpy(mem_name, optarg);
        break;
      default:
        return -1;
    }

  if (optind >= argc) {
    std::cout << "Expected argument after options\n";
    return -1;
  }

  disk_size = get_size(argv[optind]);
  disk_size = disk_size + (BLOCK_SIZE - (disk_size % BLOCK_SIZE));
  std::cout << "Disk Size: " << disk_size << "\n";

  if(file_name != NULL){
    create_file(disk_size, file_name);
    free(file_name);
  }
  if(mem_name != NULL){
    create_mem(disk_size, mem_name);
    free(mem_name);
  }
  return 0;
}

unsigned long get_size(char *size){
  long size_modifier;
  char size_format[2];
  strcpy(size_format, size + strlen(size) - 2);

  if(!strcmp(size_format, "KB"))
    size_modifier = KB;
  else if(!strcmp(size_format, "MB"))
    size_modifier = MB;
  else if(!strcmp(size_format, "GB"))
    size_modifier = GB;
  else
    size_modifier = B;

  return atoi(size) * size_modifier;
}

void create_file(unsigned long size, char *name){
  std::ofstream disk_handler(name, std::ios::binary | std::ios::out);
  disk_handler.seekp(size - 1);
  disk_handler.write("", 1);
  disk_handler.close();
  Filesystem *fs = new Filesystem(size, name, FILE_TYPE);
  fs->format();
  delete fs;
}

void create_mem(unsigned long size, char *name){
  int fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (ftruncate(fd, size) == -1)
    perror("BasicFSMem Truncate");
  void *addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED){
    perror("BasicFSMem Mmap");
    exit(-1);
  }
  Filesystem *fs = new Filesystem(size, name, MEM_TYPE);
  fs->format();
  delete fs;
}
