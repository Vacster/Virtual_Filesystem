#include "basic_fs_disk.h"

extern "C"{
  void *loadBasicFS(char *name) {
    return reinterpret_cast<void*>(new BasicFSDisk(name));
  }
}
