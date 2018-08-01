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

# step 1
runtest "0" 0
runtest "1" 1
runtest "128" 128

# step 2.1
runtest "1+1" 2
runtest "0+0" 0
runtest "64+32" 96

# step 2.2
runtest "2-1" 1
runtest "1-1" 0
runtest "64-32" 32

# step 2.3
runtest "3*5" 15
runtest "4/2" 2
runtest "3*5+1" 16
runtest "1+2*5" 11
runtest "2*3+4*5" 26
runtest "12/6-4/2" 0

echo OK
