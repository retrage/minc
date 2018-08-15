#!/bin/bash

runtest() {
    echo "$1" | ./minc > tmp.s
    cc -o tmp.exe tmp.s

    ./tmp.exe
    out=$?
    if [ "$out" != "$2" ]; then
        echo "$1: $2 expected, but got $out"
        exit 1
    fi
    echo "$1 => $2"
    rm -f tmp.{s,exe}
}

calltest() {
    echo "$1" | ./minc > tmp.s
    cc -o tmp.o -c tmp.s
    cc -o "test/$2.o" -c "test/$2.c"
    cc -o tmp.exe tmp.o "test/$2.o"

    out=`./tmp.exe`
    if [ "$out" != "$3" ]; then
        echo "$1: $3 expected, but got $out"
        exit 1
    fi
    echo "$1 => $3"
    rm -f tmp.{s,exe,o}
}

# step 1
runtest "main() { return 0; }" 0
runtest "main() { return 1; }" 1
runtest "main() { return 128; }" 128

# step 2.1
runtest "main() { return 1+1; }" 2
runtest "main() { return 0+0; }" 0
runtest "main() { return 64+32; }" 96

# step 2.2
runtest "main() { return 2-1; }" 1
runtest "main() { return 1-1; }" 0
runtest "main() { reutnr 64-32; }" 32

# step 2.3
runtest "main() { return 3*5; }" 15
runtest "main() { return 4/2; }" 2
runtest "main() { return 3*5+1; }" 16
runtest "main() { return 1+2*5; }" 11
runtest "main() { return 2*3+4*5; }" 26
runtest "main() { return 12/6-4/2; }" 0

# step 2.4
runtest "main() { return (2+5)*2; }" 14
runtest "main() { return (8-2)/3; }" 2
runtest "main() { return 25*(1+3); }" 100
runtest "main() { return 20/(5-1); }" 5
runtest "main() { return (3+4)*(5-2); }" 21
runtest "main() { return (21-1)/(7-2); }" 4

# step 2.5
runtest "main() { return 1==1; }" 1
runtest "main() { return 1==2; }" 0
runtest "main() { return 1!=1; }" 0
runtest "main() { return 1!=2; }" 1
runtest "main() { return 2+2==8/2; }" 1
runtest "main() { return 6-2==1*7; }" 0
runtest "main() { return (4-2)*(5+1)!=(4+8)/3; }" 1
runtest "main() { return (4-2)*(5+1)==(4+8)/3; }" 0

# step 3.1
runtest "main() { 1; return 2; }" 2
runtest "main() { 1+1; return 2-1; }" 1
runtest "main() { 3*5; return 4/2; }" 2
runtest "main() { 3*5+1; return 1+2*5; }" 11

# step 3.2
runtest "test_vector" 0
runtest "test_map" 0

# step 3.3
runtest "main() { a = 1; return a; }" 1
runtest "main() { b = 1 + 2; return b; }" 3
runtest "main() { a = 1; b = 2; return a + b; }" 2
runtest "main() { a = 1 * (2 + 3); b = (7 - 1) / 2; return b; }" 3
runtest "main() { a = 1 * (2 + 3); b = (7 - 1) / 2; return a * b; }" 15

# step 4.1
calltest "main() { foo(); return 0; }" "test4-1-1" "OK"
calltest "main() { bar(1, 2); return 0; }" "test4-1-2" "x=1, y=2, x+y=3"
calltest "main() { baz(1, 2, 3, 4, 5, 6); return 0; }" "test4-1-3" "21"
calltest "main() { hog(1 + 2, 3); return 0; }" "test4-1-4" "9"
calltest "main() { hog((4-2)*(5+1) != (4+8)/3, 5); return 0; }" "test4-1-4" "5"

# step 4.2
runtest "foo() { return 5; } main() { return foo(); }" 5

echo OK
