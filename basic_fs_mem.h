#include "basic_fs.h"

class BasicFSMem : public BasicFS
{
public:
  BasicFSMem(char *name, unsigned long size);
  ~BasicFSMem();
  int read_block(int block, char *buffer);
  int write_block(int block, char *buffer);
private:
  char *disk_handler;
};
