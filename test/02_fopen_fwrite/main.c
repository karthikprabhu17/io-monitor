#include <stdio.h>
#include <assert.h>

int main()
{
  FILE * f = fopen("file", "w+");
  assert(f);
  int res = fwrite("data", 2, 2, f);
  assert(res == 2);
  fclose(f);

  f = fopen("file2", "w+");
  assert(f);
  res = fprintf(f,"%s%c%c","da", 't', 'a');
  assert(res == 4);
  fclose(f);
  return 0;
}
