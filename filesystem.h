#include "basic_fs.h"
#include <bitset>
#define DISK_OFFSET 2
#define NAME_SIZE 16
#define FILE_TYPE true
#define MEM_TYPE false

struct index { //Max 128 files
  char name[NAME_SIZE];
  int block;//4
  unsigned long file_size;//12
};

struct metadata {
  char name[NAME_SIZE];
  unsigned long size;
  int bitmap_size;
  int file_amount;
};

class Filesystem {
public:
  Filesystem(unsigned long, char *, bool);
  ~Filesystem();
  void format();
  void mark_block(int block, bool used);
  void print_stats();
  bool add_file(char *name);
  bool get_file(char *name, char *copy_name);
  bool add_to_index(char *name, int block, unsigned long size);
  int get_free_block();
  unsigned long get_free_space();
  BasicFS *basicFS;
  void* sharedObj;

private:
  int bitmap_size;
  unsigned long size;
  char *name;
  bool disk_type;
};
