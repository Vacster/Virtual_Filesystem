#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "filesystem.h"

void get_file(char *file_name, char *load_name, char *copy_name, bool type);

int main(int argc, char **argv) {
  int flag;
  char *file_name = NULL, *mem_name = NULL, *load_name = NULL, *copy_name;

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

  load_name = (char*)malloc(strlen(argv[optind]));
  strcpy(load_name, argv[optind]);

  copy_name = (char*)malloc(strlen(argv[optind+1]));
  strcpy(copy_name, argv[optind+1]);

  if(file_name != NULL){
    get_file(file_name, load_name, copy_name, FILE_TYPE);
    free(file_name);
  }
  if(mem_name != NULL){
    get_file(mem_name, load_name, copy_name, MEM_TYPE);
    free(mem_name);
  }
  return 0;
}

void get_file(char *file_name, char *load_name, char* copy_name, bool type){
  Filesystem *fs = new Filesystem(0, file_name, type);
  if(fs->get_file(load_name, copy_name))
    std::cout << "File got succesfully!\n";
  else
    std::cout << "Error getting file.\n";
  delete fs;
}
