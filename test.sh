#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

#assert 0 0
#assert 42 42
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'
assert 5 'a=5;a;'
assert 100 'foo=20; bar=5; foo*bar;'
assert 5 'if(1 == 1) 5;'
assert 5 'if(1 == 1) 5; else 10;'
assert 10 'if(1 == 2) 5; else 10;'
assert 10 'a=0; while(a < 10) a = a + 1; a;'
assert 5 'a=0; b=5; while(a < 10) a = a + 1; b;'
assert 10 'for(a=0; a<10; a=a+1) a; a;'

echo OK
