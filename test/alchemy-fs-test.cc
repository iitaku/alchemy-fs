#include <sys/types.h>
#include <attr/xattr.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
  std::cout << getxattr("/tmp/fuse/hello", "hoge", NULL, 0) << std::endl;;
  return 0;
}
