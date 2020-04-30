#include <stdio.h>

int one() { return 1; }

int arg1(int n) { return n; }
int arg2(int n, int n2) { return n - n2; }
int arg3(int n, int n2, int n3) { return n - n2 - n3; }
int arg4(int n, int n2, int n3, int n4) { return n - n2 - n3 - n4; }
int arg5(int n, int n2, int n3, int n4, int n5) {
  return n - n2 - n3 - n4 - n5;
}
int arg6(int n, int n2, int n3, int n4, int n5, int n6) {
  return n - n2 - n3 - n4 - n5 - n6;
}