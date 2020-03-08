#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __DEBUG_MODE
#include "dmemory.h"

/* estrdup: 문자열을 복사하고, 에러 발생 시 종료 */
static char *estrdup(char *s)
{
    char *t;
    t = (char *) dmalloc(strlen(s) + 1);
    if (t == NULL) {
        fprintf(stderr, "estrdup(\"%.20s\") failed:", s);
        exit(2);
    }
    strcpy(t, s);
    return t;
}

enum {
    MULTIPLIER = 31,            /* 해시 상수 */
    NHASH = 4093,               /* wordtab 원소 개수 */
};

/* Word: 단어를 기록하는 해시테이블 체인 노드 */
typedef struct Word Word;
struct Word {                   
    char *text;                 /* text: 단어 */
    int  freq;                  /* freq: 출현 빈도 */
    Word *next;                 /* next: 다음 노드 */
};

static Word *wordtab[NHASH]  /* wordtab: 단어를 기록하는 해시테이블 */
    = { NULL }; 
static int nword = 0;            /* nword: 단어의 개수 */

/* hash: 문자열에 대한 해시값을 반환한다. */
static unsigned int hash(char *s)
{
    unsigned int h, i;

    h = 0;
    for (i = 0; s[i] != '\0'; i++) {
        h = h * MULTIPLIER + s[i];
    }
    return h % NHASH;
}

/* lookup: wordtab에서 text와 일치하는 Word 객체를 찾아 반환한다. 만약
 * 일치하는 원소가 없다면 새로 만든다. */
static Word *lookup(char *text)
{
    Word *p;
    char *s;
    unsigned h;
    h = hash(text);
    for (p = wordtab[h]; p != NULL; p = p->next)
        if (strcmp(text, p->text) == 0)
            return p;
    /* 찾지 못했다면 새로 만든다 */
    s = estrdup(text);
    p = (Word *) dmalloc(sizeof(Word));
    if (!p) {                   /* 할당 실패 */
        perror("lookup");
        exit(2);
    }
    p->text = s;
    p->freq = 0;
    p->next = wordtab[h];
    wordtab[h] = p;
    nword++;
    return p;
}

/* cleanup: 할당한 wordtab 원소들을 해제한다. */
static void cleanup()
{
    int i;
    Word *p, *next;
    for (i = 0; i < NHASH; i++)
        for (p = wordtab[i]; p != NULL; p = next) {
            next = p->next;
            dfree(p->text);
            dfree(p);
        }
}

/* add: 단어의 출현 빈도수를 1씩 올린다. */
static void add(char *text)
{
    Word *word;
    word = lookup(text);
    word->freq += 1;
}

/* wcmp: Word 객체들을 역순 비교한다. */
static int wcmp(const void *v1, const void *v2)
{
    Word *w1, *w2;
    w1 = *((Word **)v1);
    w2 = *((Word **)v2);
    return (w2->freq) - (w1->freq);
}

/* to_array: wordtab 원소들을 배열로 만들어 저장한다. */
void to_array(Word *words[])
{
    int i, cnt;
    Word *p;
    cnt = 0;
    for (i = 0; i < NHASH; i++)
        for (p = wordtab[i]; p != NULL; p = p->next)
            words[cnt++] = p;
}

/* rank: 출현 빈도 순으로 정리하여 상위 n개 출력한다. n이 -1이면 모두
 * 출력한다. */
static void rank(int n)
{
    Word **words;
    int i;
    words = (Word **) dmalloc (
        nword * (sizeof words[0]));
    to_array(words);
    qsort(words, nword, (sizeof words[0]), &wcmp);
    if (n < 0 || nword < n)
        n = nword;
    for (i = 0; i < n; i++)
        printf("[%03d] %s : %d times.\n",
               i+1, words[i]->text, words[i]->freq);
    dfree(words);
}

#if !defined(BUFSIZ)
#define BUFSIZ 512
#endif

/* usage: 프로그램의 사용법을 출력한다. */
static void usage(char *progname) {
    printf("usage: %s <filename> [ranking]\n", progname);
    puts("     <filename> denotes text file's name.");
    puts("     [ranking] denotes number of words to print.");
    puts("You can skip [ranking] parameter."
         "If you skip, all words will be printed.");
}
    
int main(int argc, char *argv[])
{
    char buf[BUFSIZ];
    FILE *src;
    char *endptr;
    int nprint = -1;
    if (argc < 2) {
        usage(argv[0]);
        exit(0);
    }
    if (argc == 3) {
        nprint = strtol(argv[2], &endptr, 10);
        if (endptr == argv[2] || *endptr != '\0') {
            usage(argv[0]);
            exit(0);
        }
    }

    src = fopen(argv[1], "r");
    if (!src) {
        sprintf(buf, "%s openning file %s",
                argv[0], argv[1]);
        perror(buf);
        exit(1);
    }

    while (fscanf(src, "%s", buf) == 1)
        add(buf);
    
    rank(nprint);

    cleanup();
#if defined (__DEBUG_MODE)
    /* 제대로 모두 지워졌는지 확인한다. */
    dmem_dump(stdout);
#endif
    return 0;
}

