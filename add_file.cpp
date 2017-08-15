#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "filesystem.h"

void add_file(char *file_name, char *load_name, bool type);

int main(int argc, char **argv) {
  int flag;
  char *file_name = NULL, *mem_name = NULL, *load_name = NULL;

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

  if(file_name != NULL){
    add_file(file_name, load_name, FILE_TYPE);
    free(file_name);
  }
  if(mem_name != NULL){
    add_file(mem_name, load_name, MEM_TYPE);
    free(mem_name);
  }
  return 0;
}

void add_file(char *file_name, char *load_name, bool type){
  Filesystem *fs = new Filesystem(0, file_name, type);
  if(fs->add_file(load_name))
    std::cout << "File added succesfully!\n";
  else
    std::cout << "Error adding file.\n";
  delete fs;
}
