#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <fstream>
#include "filesystem.h"

Filesystem::Filesystem(unsigned long size, char *name, bool disk_type)
:size(size),name(name),disk_type(disk_type){
  char file_name[13];
  if(disk_type == FILE_TYPE)
    strcpy(file_name, "./libFile.so");
  else
    strcpy(file_name, "./libMems.so");

  sharedObj = dlopen(file_name, RTLD_NOW);
  if(sharedObj == NULL){
    perror("SharedObj is null!\n");
    exit(-1);
  }
  dlerror();

  typedef BasicFS* (*abstractFileSystem)(char*, unsigned long);

  abstractFileSystem loadBasicFS =
    reinterpret_cast<abstractFileSystem>(dlsym(sharedObj, "loadBasicFS"));
  basicFS = (*loadBasicFS)(name, size);
  if(!size){
    struct metadata md;
    char buffer[BLOCK_SIZE];
    basicFS->read_block(1, buffer);
    memcpy(&md, buffer, sizeof(struct metadata));
    bitmap_size = md.bitmap_size;
    size = md.size;
  } else
    bitmap_size = ceil(((float)size) / BLOCK_SIZE / 8.0f / BLOCK_SIZE);
}

Filesystem::~Filesystem(){
  dlclose(sharedObj);
  delete basicFS;
}

void Filesystem::print_stats(){
  struct index *ind = (struct index*)malloc(sizeof(struct index));
  struct metadata *md = (struct metadata*)calloc(sizeof(struct metadata), 1);
  char *buffer = (char*)calloc(BLOCK_SIZE, 1);

  //Print metadata
  basicFS->read_block(1, buffer);
  memcpy(md, buffer, sizeof(struct metadata));
  std::cout << "--- METADATA ---\nName: " << md->name <<
  "\nSize: " << md->size << "\nBitmap_size: " << md->bitmap_size <<
  "\nFile Amount: " << md->file_amount << "\n";

  memset(buffer, 0, BLOCK_SIZE);
  basicFS->read_block(DISK_OFFSET+bitmap_size, buffer);

  std::cout << "--- Index ---\n";
  for(size_t x = 0; (x*sizeof(struct index)) < md->file_amount; x++){
    memset(ind, 0, sizeof(struct index));
    memcpy(ind, buffer+x, sizeof(struct index));
    std::cout << "File Name: " << ind->name <<
    "\t\tIndex Block: " << ind->block << "\t\tFile Size: " <<
    ind->file_size << "\n";
  }
}

int Filesystem::get_free_block(){
  char buffer[BLOCK_SIZE];
  for(size_t x = 0; x < bitmap_size; x++){
    basicFS->read_block(x+DISK_OFFSET, buffer);
    for(size_t y = 0; y < BLOCK_SIZE; y++)
      for(size_t z = 0; z < 8; z++)
        if(!(buffer[y] & (128 >> (7-z))))
          return (x*BLOCK_SIZE*8)+(y*8)+z;
  }
  return -1;
}
bool Filesystem::add_to_index(char *name, int block, unsigned long size){
  struct metadata md;
  struct index in;
  char *buffer = (char*)calloc(BLOCK_SIZE, 1);
  char *md_buffer = (char*)calloc(BLOCK_SIZE, 1);

  basicFS->read_block(DISK_OFFSET+bitmap_size, buffer);
  basicFS->read_block(1, md_buffer);
  memcpy(&md, md_buffer, sizeof(struct metadata));

  strcpy(in.name, name);
  in.block = block;
  in.file_size = size;

  memcpy(buffer + (sizeof(struct index) * md.file_amount),
    &in, sizeof(struct index));
  basicFS->write_block(DISK_OFFSET+bitmap_size, buffer);

  md.file_amount += 1;
  memset(md_buffer, 0, BLOCK_SIZE);
  memcpy(md_buffer, &md, sizeof(struct metadata));

  basicFS->write_block(1, md_buffer);
}

unsigned long Filesystem::get_free_space(){
  unsigned long size = ((unsigned long)get_free_block()-1) * BLOCK_SIZE * 8;
  struct metadata md;
  char *md_buffer = (char*)calloc(BLOCK_SIZE, 1);
  basicFS->read_block(1, md_buffer);
  memcpy(&md, md_buffer, sizeof(struct metadata));
  size = md.size - size;
  return size;
}

bool Filesystem::get_file(char *name, char *copy_name){
  struct index *ind = (struct index*)malloc(sizeof(struct index));
  struct metadata *md = (struct metadata*)calloc(sizeof(struct metadata), 1);
  char *buffer = (char*)calloc(BLOCK_SIZE, 1);
  int *index_1 = (int*)calloc(BLOCK_SIZE, 1);
  int *index_2 = (int*)calloc(BLOCK_SIZE, 1);
  char *data_buffer = (char*)calloc(BLOCK_SIZE, 1);
  unsigned long current_size;

  basicFS->read_block(1, buffer);
  memcpy(md, buffer, sizeof(struct metadata));
  memset(buffer, 0 , BLOCK_SIZE);
  basicFS->read_block(DISK_OFFSET+bitmap_size, buffer);
  memcpy(ind, buffer, sizeof(struct index));

  bool file_done = false;
  for(size_t x = 0; (x*sizeof(struct index)) < md->file_amount && !file_done; x ++){
    memset(ind, 0, sizeof(struct index));
    memcpy(ind, buffer+x, sizeof(struct index));
    if(!strcmp(ind->name, name)){
      std::cout << "File found!\n";
      free(md);
      free(buffer);
      std::ofstream *file_handler =
        new std::ofstream(copy_name, std::ofstream::binary | std::ofstream::out);

      if(!file_handler->is_open()){
        std::cout << "Couldn't open file.\n";
        file_handler->close();
        return false;
      }

      current_size = ind->file_size;
      memset(index_1, 0, BLOCK_SIZE);
      basicFS->read_block(ind->block, (char*)index_1);

      for(size_t y = 0; (y*sizeof(int)) < BLOCK_SIZE && !file_done; y++){//Read first index
        int index_2_block = *(index_1+y);
        std::cout << "Index_1 pointing to #" << index_2_block << "\n";
        if(index_2_block){//If there's something in the index
          memset(index_2, 0, BLOCK_SIZE);
          basicFS->read_block(index_2_block, (char*)index_2);
          for(size_t z = 0; (z*sizeof(int)) < BLOCK_SIZE; z ++){//Read second index
            int data_block = *(index_2+z);
            if(data_block){//If there's data
              std::cout << "Index_2 pointing to #" << data_block << "\n";
              memset(data_buffer, 0, BLOCK_SIZE);
              basicFS->read_block(data_block, data_buffer);
              file_handler->write(data_buffer,
                current_size < BLOCK_SIZE ? current_size : BLOCK_SIZE);
              current_size -= BLOCK_SIZE;
              if(current_size < 0){
                file_done = true;
                break;
              }
            }else
              break;
          }
        }else
          break;
      }
      free(index_1);
      free(index_2);
      free(data_buffer);
      file_handler->close();
      return true;
    }
  }
  std::cout << "File " << name << " not found.\n";
  free(md);
  free(ind);
  free(index_1);
  free(index_2);
  free(data_buffer);
  return false;

}

bool Filesystem::add_file(char *name){
  std::ifstream *file_handler =
    new std::ifstream(name, std::ifstream::binary | std::ifstream::in);

  if(!file_handler->is_open()){
    std::cout << "Error opening file. Did you write it correctly?\n";
    return false;
  }

  unsigned long file_size = file_handler->tellg();
  file_handler->seekg(0, std::ios::end);
  file_size = ((unsigned long)file_handler->tellg()) - file_size;
  std::cout << "File Size: " << file_size << "\n";
  if(file_size > get_free_space()){
    std::cout << "File is too big!\n";
    file_handler->close();
    return false;
  }
  file_handler->seekg(0, std::ios::beg);

  int files_index = DISK_OFFSET + bitmap_size + 1;

  int index_block = get_free_block();
  add_to_index(name, index_block, file_size);
  mark_block(index_block, true);

  bool file_done = false;
  //No need to read index since it is zeroed already
  int *index_buffer = (int*)calloc(BLOCK_SIZE, 1);
  for(int x = 0; (x*sizeof(int)) < BLOCK_SIZE && !file_done; x++){
    int second_index_block = get_free_block();
    int *second_index_buffer = (int*)calloc(BLOCK_SIZE, 1);
    memcpy(index_buffer+x, &second_index_block, sizeof(int));
    mark_block(second_index_block, true);

    for(int y = 0; (y*sizeof(int)) < BLOCK_SIZE; y++){//Second index
      if(file_handler->eof()){
        file_done = true;
        file_handler->close();
        break;
      }
      int data_block = get_free_block();
      memcpy(second_index_buffer+y, &data_block, sizeof(int));
      mark_block(data_block, true);

      char *buffer = (char*)calloc(BLOCK_SIZE, 1);

      file_handler->read(buffer, BLOCK_SIZE);
      basicFS->write_block(data_block, buffer);
      std::cout << "Remaining: " << file_size << "KB        \r";
      file_size -= 4096;
    }
    basicFS->write_block(second_index_block, (char*)second_index_buffer);
  }
  basicFS->write_block(index_block, (char*)index_buffer);
  return true;
}

void Filesystem::mark_block(int block, bool used){
  char buffer[BLOCK_SIZE];
  int bitmap_block = floor(block/(BLOCK_SIZE*8));
  int byte = (floor(block/8) - (BLOCK_SIZE*bitmap_block));
  basicFS->read_block(DISK_OFFSET+bitmap_block, buffer);
  if(used)
    buffer[byte] |= 1 << (block%8);
  else
    buffer[byte] &= ~(1 << (block%8));
  basicFS->write_block(DISK_OFFSET+bitmap_block, buffer);
}

void Filesystem::format(){
  char *buffer = (char*) calloc(BLOCK_SIZE, 1);
  basicFS->write_block(0, buffer);
  struct metadata md;
  strncpy(md.name, this->name, NAME_SIZE);
  md.size = this->size;
  md.bitmap_size = this->bitmap_size;
  md.file_amount = 0;
  memcpy(buffer, &md, sizeof(struct metadata));
  basicFS->write_block(1, buffer);
  free(buffer);
  buffer = (char*) calloc(BLOCK_SIZE, 1);
  for(size_t x = 0, y = 0; x < (DISK_OFFSET + bitmap_size + 1); x++){
    y = floor(x/8.0f);
    buffer[y] = buffer[y] | (1 << (x%8));
  }
  basicFS->write_block(2, buffer);
}
