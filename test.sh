#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s lib/lib.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

rm -rf lib/lib.o
cc -o lib/lib.o -c lib/lib.c

#assert 0 0
#assert 42 42
#assert 21 "5+20-4;"
#assert 41 " 12 + 34 - 5 ;"
#assert 47 '5+6*7;'
#assert 15 '5*(9-6);'
#assert 4 '(3+5)/2;'
#assert 10 '-10+20;'
#assert 0 '0==1;'
#assert 1 '42==42;'
#assert 1 '0!=1;'
#assert 0 '42!=42;'
#
#assert 1 '0<1;'
#assert 0 '1<1;'
#assert 0 '2<1;'
#assert 1 '0<=1;'
#assert 1 '1<=1;'
#assert 0 '2<=1;'
#
#assert 1 '1>0;'
#assert 0 '1>1;'
#assert 0 '1>2;'
#assert 1 '1>=0;'
#assert 1 '1>=1;'
#assert 0 '1>=2;'
#assert 10 '-10+20;'
#assert 10 '- -10;'
#assert 10 '- - +10;'
#assert 5 'a=5;a;'
#assert 100 'foo=20; bar=5; foo*bar;'
#assert 5 'if(1 == 1) 5;'
#assert 5 'if(1 == 1) 5; else 10;'
#assert 10 'if(1 == 2) 5; else 10;'
#assert 10 'a=0; while(a < 10) a = a + 1; a;'
#assert 5 'a=0; b=5; while(a < 10) a = a + 1; b;'
#assert 10 'for(a=0; a<10; a=a+1) a; a;'
#assert 5 'x=0; for(a=0; a<10; a=a+1) { if(a == 5) { x = x + 2; } if(a == 3 ) { x = x + 3; } } x;'
#assert 1 'one();'
#assert 10 'arg1(10);'
#assert 10 'arg2(15,5);'
#assert 15 'arg3(25,5,5);'
#assert 10 'arg4(25,5,5,5);'
#assert 20 'i=0; for(n=0; n<10; n=n+1) i = i + arg1(2); i;'
#assert 5 'arg5(25,5,5,5,5);'
#assert 0 'arg6(25,5,5,5,5,5);'
assert 1 'int main() { return 1; }'
assert 3 'int main() { return 2+1; }'
assert 10 'int main() { return arg1(10); }'
assert 10 'int foo() { return arg1(10); } int main() { return foo(); }'
assert 10 'int add1(a) { return 10; } int main() { return add1(10); }'
assert 10 'int add1(a) { return a; } int main() { return add1(10); }'
assert 10 'int add1(a) { return a + 1; } int main() { return add1(9); }'
assert 10 'int add(a, b) { return a + b; } int main() { return add(9,1); }'
assert 8 'int sub(a, b) { return a - b; } int main() { return sub(9,1); }'
assert 7 'int sub(a, b, c) { return a - b - c; } int main() { return sub(9,1,1); }'
assert 6 'int sub(a, b, c, d) { return a - b - c - d; } int main() { return sub(9,1,1,1); }'
assert 5 'int sub(a, b, c, d, e) { return a - b - c - d - e; } int main() { return sub(9,1,1,1,1); }'
assert 4 'int sub(a, b, c, d, e, f) { return a - b - c - d - e - f; } int main() { return sub(9,1,1,1,1,1); }'
assert 1 'int main() { return arg2(2,1); }'

echo OK
