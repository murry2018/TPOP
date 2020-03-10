while read line; do
    echo $line;
    rm -f test.c test.exe out.txt
    cat <<EOF > test.c
#include <stdio.h>
int main(void) {printf($line);}
EOF
    cc -w test.c -o test.exe
    timeout 2s ./test.exe > out.txt
    echo "[[ \`cat out.txt\` == '`cat out.txt`' ]]"
done
