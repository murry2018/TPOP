#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Nameval Nameval;
struct Nameval {
  char    *name;
  int     value;
  Nameval *next;
};

void *emalloc(size_t size) {
  void *p = malloc(size);
  if (p) {
    return p;
  } else {
    perror("malloc");
    exit(1);
  }
}

/* newitem: ... */
/* name must be a null-terminating string, and will be copied. */
Nameval *newitem(char *name, int value)
{
  Nameval *newp;

  newp = (Nameval *) emalloc(sizeof(Nameval));
  newp->name = (char *) emalloc(sizeof(char)*strlen(name));
  strcpy(newp->name, name);
  newp->value = value;
  newp->next = NULL;
  return newp;
}

/* addfront: ... */
Nameval *addfront(Nameval *listp, Nameval *newp)
{
  newp->next = listp;
  return newp;
}

/* addend: ... */
Nameval *addend(Nameval *listp, Nameval *newp)
{
  Nameval *p;
  if (listp == NULL)
    return newp;
  for (p = listp ; p->next != NULL; p = p->next)
    ;
  p->next = newp;
  return listp;
}

/* lookup: ... */
Nameval *lookup(Nameval *listp, char *name)
{
  Nameval *p;
  for (p = listp; p != NULL; p = p->next)
    if (strcmp(p->name, name) == 0)
      return p;
  return NULL;
}

/* apply: ... */
Nameval *apply(Nameval *listp,
               void (*fn)(Nameval*, void*),
               void *arg)
{
  for ( ; listp != NULL; listp = listp->next)
    (*fn)(listp, arg);
}

/* printnv: 주어진 출력 형식으로 name과 value 출력 */
void printnv(Nameval *p, void *arg)
{
  char *fmt = (char *)arg;
  printf(fmt, p->name, p->value);
}

/* inccounter: 카운터 *arg를 하나씩 늘림 */
void inccounter(Nameval *p, void *arg)
{
  int *ip = (int *)arg;
  (*ip)++;
}

/* freeall: ... */
void freeall(Nameval *listp)
{
  Nameval *next;
  for ( ; listp != NULL; listp = next) {
    next = listp->next;
    free(listp->name);
    free(listp);
  }
}

/* delitem: ... */
Nameval *delitem(Nameval *listp, char *name)
{
  Nameval *p, *prev;
  prev = NULL;
  for (p = listp; p != NULL; p = p->next) {
    if (strcmp(p->name, name) == 0) {
      if (p == listp)
        listp = p->next;
      else
        prev->next = p->next;
      free(p->name);
      free(p);
      break;
    }
    prev = p;
  }
  return listp;
}

static enum {
    NNUM = 5,         /* 테스트 배열 원소의 개수 */
};

static Nameval *make_testlist()
{
  static char *nums[NNUM] =
    {"one", "two", "three", "four", "five"};
  static Nameval *list = NULL;
  if (list == NULL)
    for (int i = NNUM-1; i >= 0; i--)
      list = addfront(list, newitem(nums[i], i+1));
  return list;
}

/* cplist: 리스트를 복사하여 돌려준다. */
Nameval *cplist(Nameval *src)
{
  Nameval *newp, *p, *psrc;
  if (src == NULL)
    return NULL;
  newp = newitem(src->name, src->value);
  p = newp;
  for (psrc = src->next; psrc != NULL; psrc = psrc->next) {
    p->next = newitem(psrc->name, psrc->value);
    p = p->next;
  }
  return newp;
}

/* append: 두 리스트를 결합한다. */
Nameval *append(Nameval *l1, Nameval *l2)
{
  Nameval *p;
  if (l1 == NULL)
    return l2;
  /* l1의 마지막 노드를 찾는다. */
  for (p = l1; p->next != NULL; p = p->next)
    ;
  p->next = l2;
  return l1;
}

/* take: 리스트를 첫 N개 원소와 그 나머지 원소들로 분리한다 */
Nameval *take(Nameval *listp, int n, Nameval **rest)
{
  Nameval *p, *prev;

  /* n번째 원소를 찾는다 */
  int i = 0;
  prev = NULL;
  for (p = listp; p != NULL; p = p->next) {
    if (i++ == n)
      break;
    prev = p;
  }
  if (prev) {
    *rest = p;
    prev->next = NULL;
  } else {
    *rest = listp;
  }
  return listp;
}

/* insert_front: 리스트에서 name을 찾아 그 앞에 원소를 추가한다. */
Nameval *insert_front(Nameval *listp, char *name, Nameval *newp)
{
  Nameval *p, *prev;
  prev = NULL;
  for (p = listp; p != NULL; p = p->next) {
    if (strcmp(p->name, name) == 0) {
      if (p == listp)
        listp = addfront(listp, newp);
      else
        prev->next = addfront(p, newp);
      break;
    }
    prev = p;
  }
  return listp;
}

/* insert_front: 리스트에서 name을 찾아 그 뒤에 원소를 추가한다. */
Nameval *insert_back(Nameval *listp, char *name, Nameval *newp)
{
  Nameval *p;
  for (p = listp; p != NULL; p = p->next) {
    if (strcmp(p->name, name) == 0) {
      newp->next = p->next;
      p->next = newp;
      break;
    }
  }
  return listp;
}

/* reverse: 리스트 연결방향을 뒤집는다. */
Nameval *reverse_rec(Nameval *listp)
{
  if (listp == NULL || listp->next == NULL)
      return listp;
  Nameval *next = listp->next;
  listp->next = NULL;
  return addend(reverse_rec(next), listp);
}


/*
p->next를 pre로
pre를 p로
p를 n으로
n을 n->next로
 */
Nameval *reverse(Nameval *listp)
{
  if (listp == NULL || listp->next == NULL)
      return listp;
  Nameval *pre, *p, *n;
  pre = NULL;
  p = listp;
  n = listp->next;
  while(p->next != NULL) {
    p->next = pre;
    pre = p;
    p = n;
    n = n->next; 
  }
  p->next = pre;
  return p;
}

void test_cplist()
{
  Nameval *list1 = make_testlist();
  Nameval *list2 = cplist(list1);
  /* 모든 원소가 메모리 상에서 다른 위치를 갖는가? */
  int same = 1;
  Nameval *p1, *p2;
  p2 = list2;
  for (p1 = list1; p1 != NULL; p1 = p1->next) {
    if (p1 != p2) {
      same = 0;
      break;
    }
    p2 = p2->next;
  }
  if (same) {
    printf("ERR: list2 is exactly same with original!\n");
    return;
  } else {
    printf("list2 is a copy of list1.\n");
  }

  printf("list1:");
  apply(list1, &printnv, "[%s %d] ");
  printf("\n");
  printf("list2:");
  apply(list2, &printnv, "[%s %d] ");
  printf("\n");
}


void test_make_testlist()
{
  Nameval *list = make_testlist();
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  Nameval *list2 = make_testlist();
  if (list == list2) {
    printf("list is a singleton.\n");
  } else {
    printf("ERR: list is not a sigleton!\n");
  }
}


void test_append()
{
  Nameval *test = make_testlist();
  Nameval *list1 = cplist(test);
  Nameval *list2 = newitem("D", 'D');
  list2 = addfront(list2, newitem("C", 'C'));
  list2 = addfront(list2, newitem("B", 'B'));
  list2 = addfront(list2, newitem("A", 'A'));
  Nameval *newhead = append(list1, list2);
  apply(newhead, &printnv, "[%s %d] ");
  printf("\n");
}

void test_take()
{
  Nameval *test = make_testlist();
  Nameval *rest;
  Nameval *til3;
  til3 = take(test, 3, &rest);
  apply(til3, &printnv, "[%s %d] ");
  printf("\n");
  apply(rest, &printnv, "[%s %d] ");
  printf("\n");
}

void test_insert_front()
{
  Nameval *test = make_testlist();
  Nameval *list = cplist(test);
  Nameval *new1 = newitem("zero", 0);
  Nameval *new2 = newitem("onepoint", 1);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  list = insert_front(list, "two", new2);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  list = insert_front(list, "one", new1);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
}

void test_insert_back()
{
  Nameval *test = make_testlist();
  Nameval *list = cplist(test);
  Nameval *new1 = newitem("zero", 0);
  Nameval *new2 = newitem("onepoint", 1);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  list = insert_back(list, "two", new2);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  list = insert_back(list, "one", new1);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
}

void test_reverse()
{
  Nameval *test = make_testlist();
  Nameval *list = cplist(test);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
  list = reverse(list);
  apply(list, &printnv, "[%s %d] ");
  printf("\n");
}
