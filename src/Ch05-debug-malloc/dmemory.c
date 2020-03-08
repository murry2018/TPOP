#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char Byte;

/* Dmem: dmalloc 할당 블록을 관리하는 리스트 노드 */
typedef struct Dmem Dmem;
struct Dmem {
    void *addr;                 /* 사용자에게 주는 메모리 */
    Byte *real;                 /* 실제 메모리 위치 */
    size_t size;                /* 할당된 실제 크기 */
    Dmem *prev, *next;          /* 이전, 다음 노드 */
};

/* memlist:메모리 블록을 관리하는 리스트 */
static Dmem *memlist = NULL;
/* memtail: memlist의 마지막 노드 */
static Dmem *memtail = NULL;

/* makemem: 새 Dmem 객체 생성. */
static Dmem *makemem(void *addr, Byte *real, size_t size)
{
    Dmem *newm = (Dmem *) malloc(sizeof(Dmem));
    if (newm) {
        newm->addr = addr;
        newm->real = real;
        newm->size = size;
        newm->prev = newm->next = NULL;
    }
    return newm;
}

/* add: memlist에 Dmem 객체 추가. 실패시 NULL */
static Dmem *add(Dmem *mem)
{
    if (!mem)
        return NULL;
    if (memlist == NULL) {
        assert(memtail == NULL);
        memlist = mem;
        memtail = mem;
    } else {
        assert(memtail != NULL);
        mem->prev = memtail;
        memtail->next = mem;
        memtail = mem;
    }
    return mem;
}

/* del: memlist에서 Dmem 객체를 삭제함. 단, 실제 객체(mem->real)는
 * 해제되지 않음. 실패시 0. */
static int del(Dmem *mem)
{
    if (!mem || !memlist)
        return 0;
    /* 리스트에서 노드를 떼어낸다 */
    if (mem->prev)
        mem->prev->next = mem->next;
    if (mem->next)
        mem->next->prev = mem->prev;
    
    if (memlist == mem)         /* 첫 노드라면 */
        memlist = memlist->next;
    if (memtail == mem)         /* 마지막 노드라면 */
        memtail = memtail->prev;
    free(mem);
    return 1;
}

/* find: memlist에서 주소가 addr인 Dmem 객체를 찾아내 그 포인터를
 * 반환한다. 실패시 NULL */
static Dmem *find(void *addr)
{
    Dmem *p;
    for (p = memlist; p != NULL; p = p->next)
        if (p->addr == addr)
            break;
    return p;
}

/* map: memlist의 원소를 하나씩 순회하며 fn을 적용한다. arg는 fn의 두
 * 번째 인자로 들어간다. */
static void map(void (*fn)(Dmem *mem, void *arg),
                void *arg)
{
    Dmem *p;
    for (p = memlist; p != NULL; p = p->next)
        (*fn)(p, arg);
}

#if defined ( __MEMLIST_TEST_MAIN )

/* tprintfn: mem을 받아 인자로 주어진 포맷 문자열로 출력한다. 포맷
 * 문자열에는 mem->addr과 mem->size가 차례로 대응된다. */
static void tprintfn(Dmem *mem, void *arg)
{
    char *fmt = (char *)arg;
    printf(fmt, mem->addr, mem->size);
}

/* makemem 함수와 tprintfn 함수를 테스트한다. */
static void test_printfn()
{
    Dmem *mem;
    mem = makemem((void *)0x1234, (Byte *)0x1235, 42);
    if (!mem) {
        perror("test_printfn");
    }
    tprintfn(mem, "%p: size %d\n");
}

/* test_memlist: 리스트 함수들을 테스트한다 */
static void test_memlist()
{
    Dmem *head2, *tail2, *res;
    Dmem *phead, *ptail;
    char *fmt = "%p: size %d\n";
    puts("=== Adding Elements ===");
    if (!(  add(makemem((void *)0x1111, (Byte *)0x1112, 42)) &&
            add(makemem((void *)0x1212, (Byte *)0x1213, 72)) &&
            add(makemem((void *)0x1234, (Byte *)0x1235, 89)) &&
            add(makemem((void *)0x6974, (Byte *)0x6975, 69)) &&
            add(makemem((void *)0x7469, (Byte *)0x7469, 74)) )) {
        perror("in test_memlist: adding elements");
        abort();
    }
    assert(memlist != NULL);
    assert(memtail != NULL);
    map(&tprintfn, fmt);

    puts("=== Finding Second Element ===");
    head2 = find((void *)0x1212);
    assert(head2 != NULL);
    assert(head2->addr == (void *)0x1212);
    tprintfn(head2, fmt);

    puts("=== Finding Second-Last Element ===");
    tail2 = find((void *)0x6974);
    assert(tail2 != NULL);
    assert(tail2->addr == (void *)0x6974);
    tprintfn(tail2, fmt);

    puts("=== Finding Non-existing Elemnt ===");
    res = find((void *)0xABCD);
    assert(res == NULL);
    if (!res)
        puts("Cannot find 0xABCD!");

    phead = memlist;
    ptail = memtail;
    
    puts("=== Deleting Second Element ===");
    del(head2); head2 = NULL;
    assert(memlist == phead);   /* 헤드 노드에 영향 x */
    assert(memtail == ptail);   /* 테일 노드에 영향 x */
    map(&tprintfn, fmt);

    puts("=== Deleting Second-last Element ===");
    del(tail2); tail2 = NULL;
    assert(memlist == phead);
    assert(memtail == ptail);
    map(&tprintfn, fmt);

    puts("=== Deleting Head Element ===");
    del(phead); phead = NULL;
    map(&tprintfn, fmt);

    puts("=== Deleting Tail Element ===");
    del(ptail); ptail = NULL;
    map(&tprintfn, fmt);

    assert(memlist == memtail);       /* 원소가 0x1234 하나만 남음 */

    puts("=== Deleting Last Element ===");
    del(memlist);
    assert(memlist == NULL);
    assert(memtail == NULL);
}

int main()
{
    test_memlist();
}

#endif
