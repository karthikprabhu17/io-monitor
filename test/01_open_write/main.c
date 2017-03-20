#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>


int main()
{
  int fd = open("file", O_WRONLY | O_CREAT, 0700);
  assert(fd > 2);
  int res = write(fd, "data", 4);
  assert(res == 4);
  res = close(fd);
  assert(!res);
  return 0;
}
