#include <stdio.h>
#include <sqlite3.h>

int main(int argc, char const *argv[])
{
  int rc = 0;
  rc = sqlite3_initialize(); // Initializes the library. If the library has already been initialized it has no effect.
  
  return 0;
}
