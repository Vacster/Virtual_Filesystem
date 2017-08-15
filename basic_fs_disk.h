#include "basic_fs.h"
#include <fstream>

class BasicFSDisk : public BasicFS
{
public:
  BasicFSDisk(char *name, unsigned long size);
  ~BasicFSDisk();
  int read_block(int block, char *buffer);
  int write_block(int block, char *buffer);
private:
  std::fstream *disk_handler;
};
