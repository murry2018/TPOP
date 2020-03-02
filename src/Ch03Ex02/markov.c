#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *emalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        perror("emalloc");
        exit(1);
    }
    return p;
}

char *estrdup(char *s) {
    int len = strlen(s);
    char *newp = (char *)emalloc((len + 1)*(sizeof s[0]));
    strcpy(newp, s);
    return newp;
}

enum {
    NPREF = 2,                  /* 접두어의 개수 */
    NHASH = 4093,               /* 해시 테이블의 버켓 개수 */
    MAXGEN = 1000,              /* 생성할 단어의 개수 */
};

typedef struct State State;
typedef struct Suffix Suffix;

/* State: 접두어/접미어 쌍 
 * 해시테이블 statetab의 버켓 리스트를 이루는 노드이다. */
struct State {
    char   *pref[NPREF];        /* 접두어 */
    Suffix *suf;                /* 접미어 */
    State  *next;               /* 다음 노드 */
};

/* Suffix: 접미어
 * State 구조체의 접미어 리스트를 이루는 노드이다. */
struct Suffix {
    char   *word;               /* 접미어 */
    Suffix *next;               /* 다음 노드 */
};

/* statetab: 접두어를 키로, 접미어 리스트를 값으로 갖는 해시테이블 */
State *statetab[NHASH];

static const int MULTIPLIER = 31;

/* hash: 문자열의 해시값을 생성한다. */
unsigned int hash(char *s)
{
    unsigned int h;

    h = 0;
    for (char *c = s; *c != '\0'; c++)
        h = MULTIPLIER * h + *c;
    return h % NHASH;
}

/* hash_pref: 접두어의 해시값을 생성한다.  */
unsigned int hash_pref(char *pref[NPREF])
{
    unsigned int h;

    h = 0;
    for (int i = 0; i < NPREF; i++)
        for (char *c = pref[i]; *c != '\0'; c++)
            h = MULTIPLIER * h + *c;
    return h % NHASH;
}

/* lookup: statetab에서 접두어를 찾아 State객체를 반환한다. 
 * create=1인 경우, 접두어가 없다면 새로 만들고 그것을 반환한다. */
State *lookup(char *prefix[NPREF], int create)
{
    unsigned int h = hash_pref(prefix);
    State *sp;
    for (sp = statetab[h]; sp != NULL; sp = sp->next) {
        int i;
        for (i = 0; i < NPREF; i++)
            if (strcmp(sp->pref[i], prefix[i]) != 0)
                break;
        if (i == NPREF)         /* 찾았다 */
            return sp;
    }
    if (create) {
        sp = emalloc(sizeof(State));
        for (int i = 0; i < NPREF; i++)
            sp->pref[i] = prefix[i];
        sp->next = statetab[h];
        sp->suf = NULL;
        statetab[h] = sp;
    }
    return sp;
}

typedef struct Word Word;
/* Word: 단어 해시셋의 원소
 * 링크드리스트의 한 노드이다. */
struct Word {
    char *word;                 /* 노드가 담고있는 단어 */
    Word *next;                 /* 링크드 리스트의 다음 노드 */
};

/* wordset: 단어들을 유일하게 저장하는 해시셋 */
Word *wordset[NHASH];

/* getword: wordset으로부터 단어를 가져온다. */
char *getword(char *word)
{
    unsigned int h;
    Word *wp;
    
    h = hash(word);
    for (wp = wordset[h]; wp != NULL; wp = wp->next)
        if (strcmp(wp->word, word) == 0)
            return wp->word;
    // 없다면 새로 만든다.
    wp = (Word*)emalloc(sizeof(Word));
    wp->word = estrdup(word);
    wp->next = wordset[h];
    wordset[h] = wp;
    return wp->word;
}

/* addsuffix: Suffix를 state에 추가 */
void addsuffix(State *sp, char *suffix)
{
    Suffix *suf;

    suf = (Suffix *) emalloc(sizeof(Suffix));
    suf->word = suffix;
    suf->next = sp->suf;
    sp->suf = suf;
}

/* add: 단어를 접미사 리스트에 추가하고, 접두사를 갱신한다. */
void add(char *prefix[NPREF], char *suffix)
{
    State *sp;

    sp = lookup(prefix, 1);     /* 없으면 생성 */
    addsuffix(sp, suffix);
    /* 접두사 배열에서 단어들을 하나씩 앞으로 당긴다 */
    memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
    prefix[NPREF-1] = suffix;
}

char NONWORD[] = "\n";          /* NONWORD: 어떤 단어도 아닌 경계값  */

/* build: 입력을 읽고 해시 테이블에 저장 */
void build(char *prefix[NPREF], FILE* f)
{
    char buf[100], fmt[10];
    int i, j;
    /* 포맷 문자열 작성, %s만 쓸 경우 buf가 오버플로우할 가능성이 있음 */
    sprintf(fmt, "%%%ds", sizeof(buf)-1);
    while (fscanf(f, fmt, buf) != EOF)
        /* getword(buf): 내용이 같은 문자열에 대해 유일한 문자열 포인터가 추가되도록 한다. */
        add(prefix, getword(buf));
}

/* generate: 한 줄에 한 단어씩 출력 생성 */
void generate(int nwords)
{
    State *sp;
    Suffix *suf;
    char *prefix[NPREF], *w;
    int i, nmatch;

    for (i = 0; i < NPREF; i++) /* 접두사 초기화 */
        prefix[i] = NONWORD;

    for (i = 0; i < nwords; i++) {
        sp = lookup(prefix, 0);
        nmatch = 0;
        for (suf = sp->suf; suf != NULL; suf = suf->next)
            if (rand() % ++nmatch == 0)
                w = suf->word;
        if (strcmp(w, NONWORD) == 0)
            break;
        printf("%s ", w);
        memmove(prefix, prefix+1, (sizeof prefix[0])*(NPREF-1));
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
    add(prefix, NONWORD);       /* 끝 표시 */
    generate(nwords);
    return 0;
}
