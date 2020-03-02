#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    NPREF = 2,         /* 접두사의 단어 수 */
    NHASH = 4093,      /* 상태를 저장할 해시 테이블 크기 */
    MAXGEN = 1000,    /* 생성할 수 있는 최대 단어 수 */
};

/* 단어는 공백을 포함하지 않으므로, 종료 신호로 충분하다. */
char NONWORD[] = "\n";

typedef struct State State;
typedef struct Suffix Suffix;

struct State {
    char   *pref[NPREF];        /* 접두어 */
    Suffix *suf;                /* 접미어 목록 */
    State  *next;               /* 해시테이블 체인 */
};

typedef struct Suffix Suffix;
struct Suffix {
    char   *word;               /* 접미어 */
    Suffix *next;               /* 다음 접미어 노드 */
};

enum {
    MULTIPLIER = 31,            /* 문자열 해시 상수 */
};

unsigned int hash(char *s[NPREF])
{
    unsigned int h;
    unsigned char *p;
    int i;

    h = 0;
    for (i = 0; i < NPREF; i++)
        for (p = (unsigned char *)s[i]; *p != '\0'; p++)
            h = MULTIPLIER * h + *p;
    return h % NHASH;
}

State *statetab[NHASH];

/* lookup: statetab에서 접두어를 검색, 필요할 경우 접두어를 생성 */
/* 접두어를 찾았거나, 만들었으면 포인터를 리턴. 그외의 경우 NULL */
/* 새 State를 만들 때 접두어의 사본을 만들지 않는다는 점에 유의하라. */
State *lookup(char *prefix[NPREF], int create)
{
    int i, h;
    State *sp;

    h = hash(prefix);
    for (sp = statetab[h]; sp != NULL; sp = sp->next) {
        for (i = 0; i < NPREF; i++)
            if (strcmp(prefix[i], sp->pref[i]) != 0)
                break;
        if (i == NPREF)         /* 찾았다면 */
            return sp;
    }
    if (create) {
        sp = (State *) malloc(sizeof(State));
        if (!sp) {
            perror("lookup");
            exit(1);
        }
          
        for (i = 0; i < NPREF; i++)
            sp->pref[i] = prefix[i];
        sp->suf = NULL;
        sp->next = statetab[h];
        statetab[h] = sp;
    }
    return sp;
}

/* estrdup: 문자열 사본 생성. 실패시 프로그램 종료*/
char *estrdup(char *str)
{
    int len = strlen(str);
    char *cp = malloc((len+1)*sizeof(char));
    if (!cp) {
        perror("build");
        exit(1);
    }
    strncpy(cp, str, len);
    cp[len] = '\0';         
    return cp;
}

void addsuffix(State *sp, char *suffix);
void add(char *prefix[NPREF], char *suffix);

/* build: 입력을 읽고 해시 테이블에 저장 */
void build(char *prefix[NPREF], FILE *f)
{
    char buf[100], fmt[10];

    /* 포맷 문자열 작성, %s만 쓸 경우 buf가 오버플로할 가능성이 있음 */
    sprintf(fmt, "%%%llus", sizeof(buf)-1);
    while(fscanf(f, fmt, buf) != EOF) {
        add(prefix, estrdup(buf));
    }
}

/* add: 단어를 접미어 리스트에 추가하고 접두어를 갱신한다 */
void add(char *prefix[NPREF], char *suffix)
{
    State *sp;

    sp = lookup(prefix, 1);     /* 없으면 생성 */
    addsuffix(sp, suffix);
    /* 접두사 배열에서 단어들을 하나씩 앞으로 당긴다. */
    memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
    prefix[NPREF-1] = suffix;
}

/* addsuffix: Suffix를 state에 추가. 접미사는 주어진 문자열의 사본이
 * 아님에 유의하라. */
void addsuffix(State *sp, char *suffix)
{
    Suffix *suf;

    suf = (Suffix *)malloc(sizeof(Suffix));
    if (!suf) {
        perror("addsuffix");
        exit(1);
    }
    suf->word = suffix;
    suf->next = sp->suf;
    sp->suf = suf;
}

/* generate: 한 줄에 한 단어씩 출력 생성 */
void generate(int nwords)
{
    State *sp;
    Suffix *suf;
    char *prefix[NPREF], *w;
    int i, nmatch;

    for (i = 0; i < NPREF; i++) /* 접두어 초기화 */
        prefix[i] = NONWORD;

    for (i = 0; i < nwords; i++) {
        sp = lookup(prefix, 0);
        nmatch = 0;
        for (suf = sp->suf; suf != NULL; suf = suf->next)
            if (rand() % ++nmatch == 0) /* 확률 = 1/nmatch */
                w = suf->word;
        if (strcmp(w, NONWORD) == 0)
            break;
        printf("%s ", w);
        memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
        prefix[NPREF-1] = w;
    }
}

/* markov main: 마르코프 체인을 이용한 텍스트 생성 */
int main(void)
{
    int i, nwords = MAXGEN;
    char *prefix[NPREF];

    for (i = 0; i < NPREF; i++)
        prefix[i] = NONWORD;
    build(prefix, stdin);
    add(prefix, NONWORD);
    generate(nwords);
    return 0;
}
