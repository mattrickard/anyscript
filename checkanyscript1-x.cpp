#line 2 "./checkanyscript1"
// vim: syntax=cpp

#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Hello from checkanyscript1, your args are:");
  for (int i = 0; i < argc; i++)
    printf(" %s", argv[i]);
  puts("");
}


