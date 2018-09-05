#!/bin/bash

runtest() {
    echo "$1" | ./minc > tmp.s
    cc -o tmp.exe tmp.s

    ./tmp.exe
    out=$?
    if [ "$out" != "$2" ]; then
        echo "$1" > tmp
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
        echo "$1" > tmp
        echo "$1: $3 expected, but got $out"
        exit 1
    fi
    echo "$1 => $3"
    rm -f tmp.{s,exe,o}
}

# step 1
runtest "int main() { return 0; }" 0
runtest "int main() { return 1; }" 1
runtest "int main() { return 128; }" 128

# step 2.1
runtest "int main() { return 1+1; }" 2
runtest "int main() { return 0+0; }" 0
runtest "int main() { return 64+32; }" 96

# step 2.2
runtest "int main() { return 2-1; }" 1
runtest "int main() { return 1-1; }" 0
runtest "int main() { return 64-32; }" 32

# step 2.3
runtest "int main() { return 3*5; }" 15
runtest "int main() { return 4/2; }" 2
runtest "int main() { return 5%3; }" 2
runtest "int main() { return 3*5+1; }" 16
runtest "int main() { return 1+2*5; }" 11
runtest "int main() { return 2*3+4*5; }" 26
runtest "int main() { return 12/6-4/2; }" 0
runtest "int main() { return 5*2+6%8/2-4; }" 9
runtest "int main() { return 1 << 2; }" 4
runtest "int main() { return 8 >> 1; }" 4
runtest "int main() { return 73 & 29; }" 9
runtest "int main() { return 73 ^ 29; }" 84
runtest "int main() { return 73 | 29; }" 93
runtest "int main() { return 0 && 0; }" 0
runtest "int main() { return 1 && 0; }" 0
runtest "int main() { return 0 && 1; }" 0
runtest "int main() { return 1 && 1; }" 1
runtest "int main() { return 0 || 0; }" 0
runtest "int main() { return 1 || 0; }" 1
runtest "int main() { return 0 || 1; }" 1
runtest "int main() { return 1 || 1; }" 1

# step 2.4
runtest "int main() { return (2+5)*2; }" 14
runtest "int main() { return (8-2)/3; }" 2
runtest "int main() { return 25*(1+3); }" 100
runtest "int main() { return 20/(5-1); }" 5
runtest "int main() { return (3+4)*(5-2); }" 21
runtest "int main() { return (21-1)/(7-2); }" 4

# step 2.5
runtest "int main() { return 1==1; }" 1
runtest "int main() { return 1==2; }" 0
runtest "int main() { return 1!=1; }" 0
runtest "int main() { return 1!=2; }" 1
runtest "int main() { return 2+2==8/2; }" 1
runtest "int main() { return 6-2==1*7; }" 0
runtest "int main() { return (4-2)*(5+1)!=(4+8)/3; }" 1
runtest "int main() { return (4-2)*(5+1)==(4+8)/3; }" 0
runtest "int main() { return (4-2)*(5+1)<(4+8)/3; }" 0
runtest "int main() { return (4-2)*(5+1)>(4+8)/3; }" 1
runtest "int main() { return (4-2)*(5+1)<=(4+8)/3; }" 0
runtest "int main() { return (4-2)*(5+1)>=(4+8)/3; }" 1

# step 3.1
runtest "int main() { 1; return 2; }" 2
runtest "int main() { 1+1; return 2-1; }" 1
runtest "int main() { 3*5; return 4/2; }" 2
runtest "int main() { 3*5+1; return 1+2*5; }" 11

# step 3.2
# runtest "test_vector" 0
# runtest "test_map" 0

# step 3.3
runtest "int main() { int a; a = 1; return a; }" 1
runtest "int main() { int b; b = 1 + 2; return b; }" 3
runtest "int main() { int a; a = 1; int b; b = 2; return a + b; }" 3
runtest "int main() { int a; a = 1 * (2 + 3); int b; b = (7 - 1) / 2; return b; }" 3
runtest "int main() { int a; a = 1 * (2 + 3); int b; b = (7 - 1) / 2; return a * b; }" 15
runtest "int main() { int a = 1 * (2 + 3); int b = (7 - 1) / 2; return b; }" 3
runtest "int main() { int a = 1 * (2 + 3); int b = (7 - 1) / 2; return a * b; }" 15
runtest "int main() { int a; a = 3; a *= 3; return a; }" 9
runtest "int main() { int a; a = 9; a /= 3; return a; }" 3
runtest "int main() { int a; a = 9; a %= 5; return a; }" 4
runtest "int main() { int a; a = 9; a += 5; return a; }" 14
runtest "int main() { int a; a = 9; a -= 5; return a; }" 4
runtest "int main() { int a; a = 1; a <<= 2; return a; }" 4
runtest "int main() { int a; a = 8; a >>= 1; return a; }" 4
runtest "int main() { int a; a = 73; a &= 29; return a; }" 9
runtest "int main() { int a; a = 73; a ^= 29; return a; }" 84
runtest "int main() { int a; a = 73; a |= 29; return a; }" 93
runtest "int main() { int a; a = 10; return +a; }" 10
runtest "int main() { int a; a = 20; return -a; }" 236
runtest "int main() { int a; a = 30; return ~a; }" 225
runtest "int main() { int a; a = 1; return !a; }" 0
runtest "int main() { int a; a = 0; return !a; }" 1
runtest "int main() { int a; a = 40; return ++a; }" 41
runtest "int main() { int a; a = 50; return --a; }" 49

# step 4.1
calltest "int main() { foo(); return 0; }" "test4-1-1" "OK"
calltest "int main() { bar(1, 2); return 0; }" "test4-1-2" "x=1, y=2, x+y=3"
calltest "int main() { baz(1, 2, 3, 4, 5, 6); return 0; }" "test4-1-3" "21"
calltest "int main() { hog(1 + 2, 3); return 0; }" "test4-1-4" "9"
calltest "int main() { hog((4-2)*(5+1) != (4+8)/3, 5); return 0; }" "test4-1-4" "5"

# step 4.2
runtest "int foo() { return 5; } int main() { return foo(); }" 5
runtest "int foo() { return 5; } int main() { return foo() + 10; }" 15
runtest "int foo() { int a; a = 1; int b; b = 2; return a + b; } int main() { return foo() + 10; }" 13
runtest "int foo() { int a; a = 1; int b; b = 2; return a + b; } int main() { int a; int b; a = 3; b = 4; return foo() * a + b; }" 13
runtest "int foo(int a) { return a; } int main() { return foo(1); }" 1
runtest "int foo(int a, int b) { int c; c = 3; return a + b + c; } int main() { int a; a = 1; int b; b = 2; return foo(a, b); }" 6
runtest "int many_args(int foo, int bar, int baz, int hog, int fuz, int hug) { baz = bar + foo * baz; hog = fuz - hug / hog; return baz + hog; } int main() { return many_args(1, 2, 3, 4, 5, 6); }" 9

# step 5
runtest "int main() { if (1) { int a; a = 1; } return a; }" 1
runtest "int main() { int a; a = 1; int b; b = 2; if (a != b) { a = 3; } else { b = 4; } return a + b; }" 5
runtest "int foo(int a) { int b; if (a == 1) b = 1; return b; } int main() { return foo(1); }" 1
runtest "int foo(int a, int b) { int res; if (a == b) res = 1; else res = 0; return res; } int main() { return foo(1, 2); }" 0
runtest "int foo(int a, int b) { int res; if (a == b) res = 1; else { res = 0; } return res; } int main() { return foo(1, 2); }" 0
runtest "int foo(int a, int b) { int res; if (a == b) { res = 1; } else res = 0; return res; } int main() { return foo(1, 2); }" 0
runtest "int foo(int a, int b) { int res; if (a == b) { res = 1; } else { res = 0; } return res; } int main() { return foo(1, 2); }" 0
runtest "int main() { int a; int b; a = 10; b = a > 5 ? 20 : 40; return b; }" 20
runtest "int main() { int a; a = 0; while (a != 10) { a = a + 1; } return a; }" 10
runtest "int main() { int a; a = 10; while (a != 0) { a = a - 1; } return a; }" 0
runtest "int main() { int a; int b; b = 0; for (a = 0; a != 10; a = a + 1) { b = b + 2; } return b; }" 20
runtest "int main() { int a; int b; b = 0; for (a = 0; a != 10; a = a + 1) b = b + 2; return b; }" 20
runtest "int fib(int n) { if (n == 0) { return 1; } if (n == 1) { return 1; } else { return fib(n - 2) + fib(n - 1); } } int main() { return fib(5); }" 8
runtest "int fib(int n) { if (n == 0) { return 1; } if (n == 1) { return 1; } else { return fib(n - 2) + fib(n - 1); } } int main() { return fib(10); }" 89
runtest "int main() { goto a; a: return 0; }" 0
runtest "int main() { int a; a = 1; if (a > 0) goto g1; else goto g2; g1: return 10; g2: return 20; }" 10
runtest "int main() { int a; a = 0; do { a += 1; } while (a < 10); return a; }" 10
runtest "int main() { int b = 0; for (int a = 0; a < 50; a += 1) { b += 2; if (a > 13) break; } return b; }" 30
runtest "int main() { int a = 0; for (int i = 0; i < 5; i += 1) { for (int j = 0; j < 5; j += 1) { a += 1; if (a > 15) break; } } return a; }" 17
runtest "int main() { int a = 0; for (int i = 0; i < 5; i += 1) { for (int j = 0; j < 5; j += 1) { a += 1; if (a > 15) break; } a += 2; } return a; }" 24

# step 6

# step 7
runtest "int main() { int x; x = 3; int *y; y = &x; return *y; }" 3

echo OK
