#include "basic_fs_disk.h"
#include <stdlib.h>

BasicFSDisk::BasicFSDisk(char *name, unsigned long size) : BasicFS(size){
  disk_handler =
    new std::fstream(name, std::fstream::binary | std::fstream::out | std::fstream::in);

  if(!disk_handler->is_open()){
    perror("Error opening disk");
    exit(-1);
  }
}
BasicFSDisk::~BasicFSDisk(){
  disk_handler->close();
}

int BasicFSDisk::read_block(int block, char *buffer){
  disk_handler->seekp(block * BLOCK_SIZE);
  disk_handler->read(buffer, BLOCK_SIZE);
}

int BasicFSDisk::write_block(int block, char *buffer){
  disk_handler->seekp(block * BLOCK_SIZE);
  disk_handler->write(buffer, BLOCK_SIZE);
}

extern "C" BasicFS *loadBasicFS(char *name, unsigned long size) {
  return new BasicFSDisk(name, size);
}
