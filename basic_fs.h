#define BLOCK_SIZE 4096

class BasicFS
{
public:
  BasicFS(unsigned long size):size(size){};
  virtual int read_block(int block, char *buffer) = 0;
  virtual int write_block(int block, char *buffer) = 0;
private:
  unsigned long size;
};
