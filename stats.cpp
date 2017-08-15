#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "filesystem.h"


int main(int argc, char **argv) {
  int flag;
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

  if(file_name != NULL){
    Filesystem *fs = new Filesystem(0, file_name, FILE_TYPE);
    fs->print_stats();
    free(file_name);
  }
  if(mem_name != NULL){
    Filesystem *fs = new Filesystem(0, file_name, MEM_TYPE);
    fs->print_stats();
    free(mem_name);
  }
  return 0;
}
