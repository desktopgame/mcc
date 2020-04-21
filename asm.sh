cc -o 9cc 9cc.c
./9cc 123 > tmp.s
cc -o tmp tmp.s
./tmp
echo $?
rm tmp tmp.s 9cc