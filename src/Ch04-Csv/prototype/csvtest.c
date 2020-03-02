#include <stdio.h>
#include <string.h>

char *unquote(char *s);

char buf[200];
char *field[20];

/* csvgetline: 한 줄씩 읽어 파싱하고 필드 개수를 리턴한다. */
/* 입력 예시: "LU",86.25,"11/4/1998","2:19PM",+4.0625 */
int csvgetline(FILE *fin)
{
    int nfield;
    char *p, *q;
    if (fgets(buf, sizeof(buf), fin) == NULL)
        return -1;
    nfield = 0;
    for (q = buf; (p=strtok(q, ",\n\r")) != NULL; q = NULL)
        field[nfield++] = unquote(p);
    return nfield;
}

/* unquote: 여는 따옴표와 닫는 따옴표를 제거 */
char *unquote(char *s)
{
    if (s[0] == '"') {
        if (s[strlen(s)-1] == '"')
            s[strlen(s)-1] = '\0';
        s++;
    }
    return s;
}

/* csvtest main: csvgetline 함수 테스트 */
int main(void)
{
    int i, nf;

    while ((nf = csvgetline(stdin)) != -1)
        for (i = 0; i < nf; i++)
            printf("field[%d] = '%s'\n", i, field[i]);
    return 0;
}
