#!/bin/bash

c=0
prevcode=""
while read line; do
    if [[ $line =~ ^[[:space:]]*$ ]]; then # 빈 라인이라면
        continue
    fi
    if [[ $line =~ ^[[:space:]]*#.*$ ]]; then # 주석이 있다면
        continue
    fi
    ((c++))
    if (( c % 2 )); then
        rm -f out.txt test.c test.exe   # 기존 출력 제거
        cat <<EOF > test.c              # 코드 작성
#include <stdio.h>
int main() { printf($line); }
EOF
        cc -w test.c -o test.exe        # 컴파일
        timeout 2s ./test.exe > out.txt # 실행시간 2초 제한
        prevcode=$line
    else
        if ! eval $line; then
            echo "BAD:" $prevcode
        fi
    fi
done < input.txt
