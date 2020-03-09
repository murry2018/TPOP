#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* FMTLEN: 한 행에 출력되는 최대 문자 수 */
int FMTLEN = 80;
/* rowlen: 한 행에 출력된 문자 수 */
int rowlen = 0;

/* vischar: 1바이트 문자 c를 출력한다. 비출력 문자는 제거한다. */
void vischar(unsigned char c, FILE *fout)
{
    if (isprint(c) || isspace(c)) {
        fputc(c, fout);
        if (c=='\n' || c=='\r')
            rowlen = 0;
        else
            rowlen++;
    } // else do nothing
}

/* vis: 입력(fin)으로 들어온 문자열을 모두 출력하되, 비출력 문자는
 * 제거하고, 행 길이가 FMTLEN을 넘으면 다음행으로 넘긴다. */
void vis(FILE* fin)
{
    char c;
    rowlen = 0;
    while ((c = fgetc(fin)) != EOF) {
        vischar(c, stdout);
        if (rowlen >= FMTLEN) {
            putchar('\n');
            rowlen = 0;
        }
    }
}

void usage(char *progname) {
    fprintf(stderr, "usage: %s [-fDD filename]*\n"
            "DD denotes an positive integer.\n", progname);
    exit(2);
}

int main(int argc, char *argv[])
{
    int i, fmtlen, err;
    FILE *fin;
    char buf[BUFSIZ], *strptr, *endptr;
    if (argc == 1)
        usage(argv[0]);
    for (i = 1; i < argc; i++) {
        if (strncmp("-f", argv[i], 2) == 0) {
            strptr = &(argv[i][2]);
            fmtlen = strtol(strptr, &endptr, 10);
            if (strptr != endptr)
                FMTLEN = fmtlen;
            else
                err = 1;
            if (FMTLEN < 1 || err) {
                fprintf(stderr,
                        "reading arg[%d]: invalid fmt value '%s'\n",
                        i, strptr);
                usage(argv[0]);
            }
            continue;
        }
        if ((fin = fopen(argv[i], "rb")) == NULL) {
            strcpy(buf, "opening ");
            strcat(buf, argv[i]);
            perror(buf);
            usage(argv[0]);
        } else {
            vis(fin);
            fclose(fin);
        }
    }
    return 0;
}
