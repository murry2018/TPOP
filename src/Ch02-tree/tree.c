#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if  defined( __linux__ )
  #include <sys/time.h>		/* gettimeofday */
#elif defined( _WIN32 )
  #include <Windows.h>		/* GetTickCount */
#endif
#include <time.h>

#include <assert.h>             /* assert */

#define assert_nvequal(X, Y) {                  \
  if ((X) != NULL && (Y) == NULL ||		\
      (X) == NULL && (Y) != NULL ) {		\
    assert( (X) == (Y) );	                \
  } else if ((X) != (Y)) {			\
    assert(strcmp((X)->name, (Y)->name) == 0);	\
    assert( (X)->value == (Y)->value );		\
    assert( (X)->left  == (Y)->left  );		\
    assert( (X)->right == (Y)->right );		\
  }                                             \
}


/* get_elapsed: 주어진 함수의 실행시간을 잰다. */
double get_elapsed(void (*fn)(void*), void* arg)
{
  double elapsed_time;
#if defined ( __linux__ )
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  (*fn)(arg);
  gettimeofday(&t2, NULL);

  elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000.0; /* sec to ms */
  elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000.0; /* us to ms */
#elif defined ( _WIN32 )
  DWORD t1, t2;
  t1 = GetTickCount();
  (*fn)(arg);
  t2 = GetTickCount();

  elapsed_time = (double) t2 - (double) t1;
#endif
  return elapsed_time;
}

typedef struct Nameval Nameval;
struct Nameval {
  char *name;
  int  value;
  Nameval *left;    /* lesser */
  Nameval *right;   /* greater */
};

/* newitem: 새로운 Nameval 노드를 만듬 */
Nameval *newitem(char *name, int value)
{
  Nameval *x = (Nameval *)malloc(sizeof(Nameval));
  x->name = name;
  x->value = value;
  x->left = NULL;
  x->right = NULL;
  return x;
}

/* insert: newp를 treep에 추가하고 treep를 리턴 */
Nameval *insert(Nameval *treep, Nameval *newp)
{
  int cmp;
  if (treep == NULL)
    return newp;
  cmp = strcmp(newp->name, treep->name);
  if (cmp == 0)
    /* weprintf("insert: duplicated entry %s ignored", */
    /*          newp->name); */
    fprintf(stderr, "insert: duplicated entry %s ignored.\n",
            newp->name);
  else if (cmp < 0)
    treep->left = insert(treep->left, newp);
  else
    treep->right = insert(treep->right, newp);
  return treep;
}

/* lookup: treep 트리에서 name 검색 */
Nameval *lookup(Nameval *treep, char *name)
{
  int cmp;

  if (treep == NULL)
    return NULL;
  cmp = strcmp(name, treep->name);
  if (cmp == 0)
    return treep;
  else if (cmp < 0)
    return lookup(treep->left, name);
  else
    return lookup(treep->right, name);
}


/* nrlookup: 비재귀적으로 treep 트리에서 name 검색 */
Nameval *nrlookup(Nameval *treep, char *name)
{
  int cmp;

  while (treep != NULL) {
    cmp = strcmp(name, treep->name);
    if (cmp == 0)
      return treep;
    else if (cmp < 0)
      treep = treep->left;
    else
      treep = treep->right;
  }
  return NULL;
}

/* applyinorder: treepㅇ fn을 중위 순회법으로 적용 */
void applyinorder(Nameval *treep,
                  void (*fn)(Nameval*, void*),
                  void *arg)
{
  if (treep == NULL)
    return;
  applyinorder(treep->left, fn, arg);
  (*fn)(treep, arg);
  applyinorder(treep->right, fn, arg);
}

/* applypostorder: treep에 fn을 후위 순회로 적용 */
void applypostorder(Nameval *treep,
                    void (*fn)(Nameval*,void*),
                    void *arg)
{
  if (treep == NULL)
    return;
  applypostorder(treep->left, fn, arg);
  applypostorder(treep->right, fn, arg);
  (*fn)(treep, arg);
}

void printnv(Nameval *treep, void *s)
{
  char *fmt;
  fmt = (char *)s;
  printf(fmt, treep->value, treep->name);
}

enum {
      NITEM = 10000000,           /* 트리 원소 개수 */
      LEN_DEFAULT = 10,          /* 이름 길이 기본값 */
      LEN_VAR     = 6,          /* 이름 길이 가변값(0보다 커야함) */
};

/* random_name: 영어 소문자로 된 랜덤한 문자열을 만든다. */
char *random_name()
{
  int nalpha = 'z' - 'a' + 1;
  int len = LEN_DEFAULT;
  len += rand()%LEN_VAR;
  
  char *name = malloc((len+1)*sizeof(char));
  for (int i = 0; i < len; i++)
    name[i] = rand() % nalpha + 'a';
  name[len] = '\0';
  
  return name;
}

/* make_testtree: NITEM개의 임의의 원소를 갖는 트리를 만들어 돌려주고,
   마지막으로 만들어진 이름을 endname에 담는다. */
Nameval *make_testtree(char **endname)
{
  static Nameval *treep = NULL;
  static char *end = NULL;
  if (treep == NULL) {
    char *name;
    printf("Generating tree...");
    for (int i = 0; i < NITEM; i++) {
      name = random_name();
      treep = insert(treep, newitem(name, i));
    }
    end = name;
    printf("Done.\n");
  }
  *endname = end;
  return treep;
}

/* 성능 테스트용 lookup 함수 래퍼 */
void wrap_lookup(void *arg)
{
  Nameval *treep = ((Nameval**)arg)[0];
  char    *name  = (char*)(((Nameval**)arg)[1]);
  Nameval *x;
  printf("finding %s... ", name);
  x = lookup(treep, name);
  if (x && strcmp(x->name, name) == 0)
    printf("GOOD\n");
  else
    printf("BAD\n");
}

/* 성능 테스트용 nrlookup 함수 래퍼 */
void wrap_nrlookup(void *arg)
{
  Nameval *treep = ((Nameval**)arg)[0];
  char    *name  = (char*)(((Nameval**)arg)[1]);
  Nameval *x;
  printf("finding %s... ", name);
  x = nrlookup(treep, name);
  if (x && strcmp(x->name, name) == 0)
    printf("GOOD\n");
  else
    printf("BAD\n");
}

void test_lookupfns()
{
  srand(time(NULL));
  char    *name;
  Nameval *testp  = make_testtree(&name);
  Nameval *arg[2] = {testp, (Nameval *)name};
  double  elapsed = get_elapsed(&wrap_lookup, arg);
  printf("lookup   :: elapsed: %f ms.\n", elapsed);
  elapsed = get_elapsed(&wrap_nrlookup, arg);
  printf("nrlookup :: elapsed: %f ms.\n", elapsed);
}

/* 빈 트리에 삽입하는 경우 */
void test_insert_1()
{
  Nameval *treep = NULL;
  Nameval *head = newitem("5", 5);
  treep = insert(treep, head);
  assert_nvequal(head, treep);
}


/* 트리의 빈 왼쪽 트리에 삽입하는 경우 */
void test_insert_2()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  Nameval *lnode = newitem("4", 5);
  treep = insert(treep, lnode);
  assert_nvequal(treep->left, lnode);
}

/* 트리의 빈 오른쪽 트리에 삽입하는 경우 */
void test_insert_3()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  Nameval *rnode = newitem("6", 6);
  treep = insert(treep, rnode);
  assert_nvequal(treep->right, rnode);
}

/* 왼쪽 자식 트리의 빈 오른쪽 링크에 삽입하는 경우 */
void test_insert_4()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  treep = insert(treep, newitem("3", 3));
  Nameval *lrnode = newitem("4", 4);
  treep = insert(treep, lrnode);
  assert_nvequal(treep->left->right, lrnode);
}

/* 중복된 원소 삽입: 단정문으로 확인할 수 없음! */
void test_insert_5()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  treep = insert(treep, newitem("5", 5)); /* err1 */
  treep = insert(treep, newitem("3", 3));
  treep = insert(treep, newitem("3", 3)); /* err2 */
}

/* test_insert: 삽입 테스트 */
void test_insert()
{
  test_insert_1();
  test_insert_2();
  test_insert_3();
  test_insert_4();
  test_insert_5();
}

/* 루트 노드의 원소를 찾는 경우 */
void test_lookup_1()
{
  Nameval *treep = NULL;
  Nameval *root = newitem("5", 5);
  treep = insert(treep, root);

  Nameval *search;
  search = lookup(treep, "5");
  assert_nvequal(search, root);
}

/* 일반적인 탐색 */
void test_lookup_2()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  treep = insert(treep, newitem("3", 3));
  treep = insert(treep, newitem("4", 4));
  treep = insert(treep, newitem("2", 2));
  Nameval *target = newitem("6", 6);
  treep = insert(treep, target);
  treep = insert(treep, newitem("9", 9));
  treep = insert(treep, newitem("7", 7));
  treep = insert(treep, newitem("8", 8));
  Nameval *search;
  search = lookup(treep, "6");
  assert_nvequal(search, target);
}

/* 없는 원소 탐색 */
void test_lookup_3()
{
  Nameval *treep = NULL;
  treep = insert(treep, newitem("5", 5));
  treep = insert(treep, newitem("3", 3));
  treep = insert(treep, newitem("4", 4));
  treep = insert(treep, newitem("2", 2));
  treep = insert(treep, newitem("6", 6));
  treep = insert(treep, newitem("9", 9));
  treep = insert(treep, newitem("7", 7));
  treep = insert(treep, newitem("8", 8));
  Nameval *search;
  search = lookup(treep, "1");
  assert_nvequal(search, (Nameval*)NULL);
}

/* test_lookup: 탐색 테스트 */
void test_lookup()
{
  test_lookup_1();
  test_lookup_2();
  test_lookup_3();
}

int main(void)
{
  test_lookup();
  return 0;
}
