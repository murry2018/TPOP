#include <stdio.h>
#include <stdlib.h>

char line[1024];

void random_string(char s[], int size)
{
    int i;
    int len = size-1;
    static const int nalpha = 'z'-'a'+1;
    for (i = 0; i < len; i++)
        s[i] = rand() % nalpha + 'a';
    s[len] = ',';               /* 구두점 */
}

char *genline()
{
    char *s = line;
    /* 10개의 필드 생성 */
    for (int i = 0; i < 10; i++) {
        /* 각 토큰의 길이는 10바이트 */
        random_string(s, 10);
        s += 10;
    }
    *(s-1) = '\0';
    return line;
}

int main()
{
    /* 10만줄 생성 */
    for (int i=0; i < 100000; i++)
        printf("%s\n", genline());
}
