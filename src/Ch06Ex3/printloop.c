/* 실행 방법: [key freq ]* 형식의 파일을 준비하여 입력으로 넣는다.  */
#include <stdio.h>

int main(void)
{
    int ascii_key, nelem, i;
    while (scanf("%d %d", &ascii_key, &nelem) == 2)
        for (i = 0; i < nelem; i++)
            putchar(ascii_key);
}
