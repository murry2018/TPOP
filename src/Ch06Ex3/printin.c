/* 실행 방법: [begin end ]* 형식의 입력을 넣는다. */
#include <stdio.h>

int main(void)
{
    int begin, end;
    char c;
    while (scanf("%d %d", &begin, &end) == 2) {
        if (begin <= end)
            for (c = begin; c <= end; c++)
                putchar(c);
        else
            for (c = begin; c >= end; c--)
                putchar(c);
    }
}
