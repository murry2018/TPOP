#include "csv.h"
#include <stdlib.h>
#include <string.h>
enum {
    NOMEM = -2,                 /* 메모리 부족을 표시하는 시그널 */
    INIT_SIZE = 1,              /* 메모리블록 초기 할당 크기 */
    INCREASE_FACTOR = 2,        /* 재할당시 증가 배수 */
};

static char *line = NULL;       /* 입력된 문자 데이터 */
static char *sline = NULL;      /* split이 쓸, line의 복사본 */
static int maxline = 0;         /* line[]과 sline[]의 크기 */
static char **field = NULL;     /* 필드를 나타내는 포인터들 */
static int maxfield = 0;        /* field[]의 크기(capacity) */
static int nfield = 0;          /* field[]에 저장된 필드 개수 */
static int is_new = 0; /* 새로운 데이터가 들어왔는가? 들어왔다면 split()해야할것이다... */

static char fieldsep[] = ",";   /* 필드 구분에 쓰일 구분자 */

static void reset(void);
static int endofline(FILE *fin, int c);
static int split(void);
static char *advquoted(char *p);

/* csvgetline: 한 줄 입력 받음, 필요하면 자동으로 메모리 추가 할당함 */
/* 입력 예시: "LU",86.25,"11/4/1998","2:19PM",+4.0625 */
char *csvgetline(FILE *fin)
{
    int i, c;
    char *newl, *news;
    if (line == NULL) {         /* 첫 호출시 메모리 할당 */
        maxline = maxfield = INIT_SIZE;
        line = (char *) malloc(maxline);
        sline = (char *) malloc(maxline);
        field = (char **) malloc(maxfield*sizeof(field[0]));
        if (line == NULL || sline == NULL || field == NULL) {
            reset();
            return NULL;        /* 메모리 부족 */
        }
    }
    for (i=0;(c=fgetc(fin)) != EOF && !endofline(fin, c); i++) {
        if (i >= maxline-1) {   /* line을 확장한다. */
            maxline *= INCREASE_FACTOR;
            newl = (char *) realloc(line, maxline);
            news = (char *) realloc(sline, maxline);
            line = newl;
            sline = news;
            if (newl == NULL || news == NULL) {
                reset();
                return NULL;    /* 메모리 부족 */
            }
        }
        line[i] = c;
    }
    line[i] = '\0';
    if (c == EOF && i == 0)
        return NULL;
    is_new = 1;
    return line;
}

/* reset: 변수를 초기 상태로 만든다. */
static void reset(void)
{
    free(line);                 /* ANSI C 기준으로 free(NULL) 가능 */
    free(sline);
    free(field);
    line = NULL;
    sline = NULL;
    field = NULL;
    maxline = maxfield = nfield = 0;
}

/* endofline: 검사후 \r, \n, \r\n 또는 EOF를 없앤다 */
static int endofline(FILE *fin, int c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if (c == '\r') {
        c = getc(fin);
        if (c != '\n' && c != EOF)
            ungetc(c, fin);     /* line separator가 \r인 경우 더 읽은걸 도로 집어넣는다 */
    }
    return eol;
}

/* split: 입력 줄을 필드로 구분한다. */
static int split(void)
{
    char *p, **newf;
    char *sepp;                 /* 임시 구분자에 대한 포인터 */
    int sepc;                   /* 임시 구분자 */

    nfield = 0;
    if (line[0] == '\0')
        return 0;
    strcpy(sline, line);
    p = sline;

    do {
        if (nfield >= maxfield) {
            maxfield *= INCREASE_FACTOR;
            newf = (char **) realloc(field,
                        maxfield * (sizeof field[0]));
            if (newf == NULL) {
                return NOMEM;
            }
            field = newf;
        }
        if (*p == '"')
            sepp = advquoted(++p); /* 시작하는 따옴표는 넘긴다 */
        else
            sepp = p + strcspn(p, fieldsep);
        sepc = sepp[0];
        sepp[0] = '\0';         /* 필드를 자른다. */
        field[nfield++] = p;
        p = sepp + 1;
    } while (sepc == ',');

    return nfield;
}

/* advquoted: 따옴표로 둘러싼 필드의 따옴표를 제거하고 다음 구분자를
 * 가리키는 포인터를 리턴한다. */
static char *advquoted(char *p)
{
    int i, j;
    for (i = j = 0; p[j] != '\0'; i++, j++) {
        if (p[j] == '"' && p[++j] != '"') {
            /* 다음 구분자나 \0이 나오는 부분까지 복사한다 */
            int k = strcspn(p+j, fieldsep);
            memmove(p+i, p+j, k);
            i += k;
            j += k;
            break;
        }
        p[i] = p[j];
    }
    p[i] = '\0';
    return p + j;
}

/* csvfield: n번째 필드에 대한 포인터를 리턴한다. */
char *csvfield(int n)
{
    if (n < 0 || n >= nfield)
        return NULL;
    if (is_new) {
        if (split() == NOMEM) {
            reset();
            return NULL;
        }
    }
    return field[n];
}

/* csvnfield: 필드 개수를 리턴한다. */
int csvnfield(void)
{
    if (is_new) {
        if (split() == NOMEM) {
            reset();
            return 0;
        }
    }
    return nfield;
}
