#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct Sequence Sequence;
struct Sequence {
  char **data;      /* array of strings */
  int nval;
  int capa;
};

enum {
  DEFAULT_CAPA = 1,         /* Sequence 기본 크기 */
  INCREASE_FAC = 2,         /* Sequence 크기 증가 배수 */
};

/* add_seq: seq의 뒤에 원소를 하나 추가함(O(1)) */
Sequence *add_seq(Sequence *seq, char *data)
{
  /* 빈 시퀀스일때 새로 만듬 */
  if (seq == NULL) {
    Sequence *newp;
    newp = (Sequence*)malloc(sizeof(Sequence));
    newp->nval = 0;
    newp->capa = DEFAULT_CAPA;
    newp->data = (char**)malloc(newp->capa *
                                (sizeof(char*)));
    seq = newp;
  }
  /* 가득 찬 경우 INCREASE_FAC 배로 늘림 */
  if (seq->nval+1 == seq->capa) {
    char **newdata;
    seq->capa *= INCREASE_FAC;
    newdata = (char**)realloc(seq->data, seq->capa *
                              (sizeof(char*)));
    if (newdata == NULL) {
      perror("add_seq");
      exit(1);
    }
    seq->data = newdata;
  }
  
  seq->data[seq->nval++] = data;
  return seq;
}

typedef struct Nameval Nameval;
struct Nameval {
  char *name;
  Sequence *seq;
};

enum {
      TABSIZ = 1024,   /* nametab 크기 */
};
Nameval *nametab[TABSIZ];
int nval = 0;

/* new_nameval: 새 Nameval 개체를 만듬 */
Nameval *new_nameval(char *name)
{
  Nameval *newp;
  newp = (Nameval*)malloc(sizeof(Nameval));
  newp->name = name;
  newp->seq = NULL;
  return newp;
}

/* additem: nametab의 name에 val 추가 */
int additem(char *name, char *val)
{
  for (int i = 0; i < nval; i++) {
    if (strcmp(name, nametab[i]->name) == 0) {
      nametab[i]->seq =
        add_seq(nametab[i]->seq, val);
      return i;
    }
  }
  if (nval+1 < TABSIZ) {
    nametab[nval] = new_nameval(name);
    nametab[nval]->seq =
      add_seq(nametab[nval]->seq, val);
    ++nval;
    return nval-1;
  } else {
    fprintf(stderr, "nametab이 꽉 찼습니다.");
    return -1;
  }
}

/* parse_string: aaaNN 꼴 문자열을 분리 */
void parse_string(char *s, char **pname, char **pval)
{
  int i;
  /* id부분의 길이를 구해서 i라고 둔다. */
  for (i = 0; s[i] != '\0'; i++)
    if (isdigit(s[i]))
      break;
  char *name = (char*)malloc((i+1)*sizeof(s[0]));
  strncpy(name, s, i);
  name[i] = '\0';

  int vlen = strlen(s+i);
  char *value = (char*)malloc((vlen+1)*sizeof(s[0]));
  value = strncpy(value, s+i, vlen);
  value[vlen] = '\0';
  
  *pname = name;
  *pval  = value;
}

/* dump: nametab을 json 형태로 출력 */
void dump(FILE *out)
{
  fputs("[\n", out);
  for (int i = 0; i < nval; i++) {
    fprintf(out, "    {\n        id: '%s',\n        sequence:",
            nametab[i]->name);
    int seqsz = nametab[i]->seq->nval;
    char **data = nametab[i]->seq->data;
    for (int j = 0; j < seqsz; j++) {
      fprintf(out, " '%s'%c", data[j],
              (j+1 == seqsz ? '\n' : ','));
    }
    fprintf(out, "    }%s\n",
            (i+1 == nval ? "" : ","));
  }
  fputs("]\n", out);
}

#ifndef BUFSIZ
  #define BUFSIZ 512
#endif

int main(void)
{
  char buf[BUFSIZ];
  while (scanf("%s", buf) == 1) {
    char *name, *val;
    parse_string(buf, &name, &val);
    additem(name, val);    
  }
  dump(stdout);
  /* 해제는 시스템에 맡긴다. */
  return 0;
}
