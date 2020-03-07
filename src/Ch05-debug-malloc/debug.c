#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char Byte;

#define CHECKBYTE ((Byte)0xff)

/* dmem은 리스트의 노드이다 */
typedef struct dmem dmem;
struct dmem {                   /* dmalloc으로 할당한 메모리 정보 */
    void *p;                    /* 할당 위치(검증 바이트 포함) */
    size_t sz;                  /* 할당 크기(검증 바이트 포함) */
    dmem *next;                 /* 다음 노드 */
};

static dmem *makemem(void *p, size_t sz);
static int push(void *p, size_t sz);
static int delete(void *addr);
static dmem *find(void *addr);
static const char *fmt_pointer();
static const char *fmt_space();

dmem *head = NULL,              /* 리스트 첫 노드 */
    *tail = NULL;               /* 리스트 마지막 노드 */

/* makemem: mem 객체를 생성한다. */
static dmem *makemem(void *p, size_t sz)
{
    dmem *newmem;
    newmem = (dmem *) malloc(sizeof(dmem));
    if (newmem) {
        newmem->p = p;
        newmem->sz = sz;
        newmem->next = NULL;
    }
    return newmem;
}

/* push: 리스트에 mem 객체를 추가한다. 성공시 1, 실패시 0 반환. */
static int push(void *p, size_t sz)
{
    dmem *mem = makemem(p, sz);
    if (!mem)                   /* dmem 객체 할당 실패 */
        return 0;
    if (head == NULL) {         /* 리스트가 비었을 때 */
        assert(tail == NULL);
        head = mem;
        tail = mem;
    } else {
        assert(tail != NULL);
        tail->next = mem;
        tail = mem;
    }
    return 1;
}

/* delete: 리스트에서 주소가 addr인 객체를 찾아 지운다. 성공시 1, 실패시 0 반환 */
static int delete(void *addr)
{
    dmem *cur, *pre;
    if (head == NULL) {         /* 노드가 없음 */
        assert (tail == NULL);
        return 0;
    }
    pre = NULL;
    for (cur = head; cur != NULL; cur = cur->next) {
        if (cur->p == addr) {
            if (pre)            /* 일반적 경우 */
                pre->next = cur->next;
            else                /* 첫 노드라면 */
                head = cur->next;
            if (!cur->next)     /* 마지막 노드라면 */
                tail = pre;
            free(cur);
            return 1;
        }
        pre = cur;
    }
    return 0;                   /* 주소가 addr인 객체를 찾지 못함 */
}

/* find: 리스트에서 주소가 addr인 객체를 찾아낸다. 실패시 NULL 반환 */
static dmem *find(void *addr)
{
    dmem *cur;
    for (cur = head; cur != NULL; cur = cur->next)
        if (cur->p == addr)
            break;
    return cur;
}

/* fmt_pointer: 포인터 형식의 포매터를 만들어낸다. */
static const char *fmt_pointer()
{
    static const char *fmt = "0x%%0%dx";
    static       char buf[32] = "";
    if (buf[0] == '\0')
        sprintf(buf, fmt, 2*sizeof(void *));
    return (const char *)buf;
}

/* fmt_space: 포인터 형식의 길이에 맞춘 스페이스 열을 만들어 낸다. */
static const char *fmt_space()
{
    static const int size = 2 * (sizeof(void *)) + 2;
    static char buf[32] = "";
    if (buf[0] == '\0') {
        int i;
        for (i = 0; i < size; i++) {
            buf[i] = ' ';
        }
        buf[i] = '\0';
    }
    return (const char *)buf;
}

#ifdef __DEBUG_TEST_MAIN

/* test_print_item: 하나의 mem을 출력한다. */
static void test_print_item(dmem *mem, FILE *fout)
{
    fprintf(fout, fmt_pointer(), mem->p);
    fprintf(fout, "\n");
    fprintf(fout, fmt_space());
    fprintf(fout, "size: %d\n", mem->sz);
}

/* test_print_all: 모든 노드를 출력한다. */
static void test_print_all(FILE *fout)
{
    dmem *cur;
    for (cur = head; cur != NULL; cur = cur->next)
        test_print_item(cur, fout);
}

/* makemem, fmt_pointer, fmt_space를 테스트한다. */
static void test_utils()
{
    dmem *mem = makemem((void *)0x1234, 42);
    assert(mem->next == NULL);
    test_print_item(mem, stdout);
}

/* is_clean: mem 객체의 무결성을 검증한다. 데이터 침범이 없으면 1,
 * 있으면 0을 반환한다. */
static Byte *is_clean(dmem *mem)
{
    Byte *asbytes = (char *)mem->p;
    return (asbytes[0] == CHECKBYTE) &&
        (asbytes[mem->sz-1] == CHECKBYTE);
}

/* to_daddr: dmalloc에서 내주는 주소로부터 실제 할당 위치를 찾는다.
 * dmalloc에서 내주는 주소는 실제 할당위치에 +1바이트 더한 값이다. */
static Byte *to_daddr(void *p) {
    Byte *asbytes = (Byte *)p;
    return p-1;
}

/* dmalloc: 디버그를 위한 할당 함수. 메모리 넘침이 감지되면
 * 에러메시지를 출력하고 종료한다. */
void *dmalloc(size_t n)
{
    void *newp;
    Byte *asbytes;
    size_t sz;
    assert(sizeof(Byte) == 1);
    /* TODO: 메모리 넘침 감지 */
    sz = n+2;                   /* 앞 뒤 검증 바이트 추가 */
    newp = malloc(sz);   
    if (!newp)
        return NULL;
    asbytes = (char *)newp;
    asbytes[0] = CHECKBYTE;
    asbytes[sz-1] = CHECKBYTE;
    push(newp, sz);
    return (void *)(&asbytes[1]);
}

void dfree(void *p)
{
    Byte *daddr = to_daddr(p);
    /* TODO: 메모리 넘침 감지 */
    if (!delete((void *)daddr)) {
        /* TODO: 에러메시지를 출력함 */
        abort();
    }
    
}

/* dmem을 위한 함수들(push, delete, find)를 테스트한다. */
static void test_dmem()
{
    dmem *result;
    if (!(  push((void *)0x1111, 42) &&
            push((void *)0x1212, 43) &&
            push((void *)0x1234, 44) &&
            push((void *)0x6974, 69) &&
            push((void *)0x7469, 74) )) {
        perror("test_dmem");
        abort();
    }
    puts("=== PUSH DONE ===");
    assert(head != NULL);
    test_print_all(stdout);
    
    result = find((void *)0x7469);
    puts("=== FIND LAST(0x7469) NODE ===");
    assert(head != NULL);
    assert(result != NULL);
    assert(tail == result);
    test_print_item(result, stdout);

    delete((void *)0x1212);
    puts("=== DELTE 0x1212 NODE ===");
    assert(head != NULL);
    assert(tail == result);
    test_print_all(stdout);

    delete((void *)0x1111);
    puts("=== DELETE HEAD NODE ===");
    assert(head != NULL);
    assert(tail == result);
    test_print_all(stdout);

    result = find((void *)0x6974);
    puts("=== FIND 0x6974 NODE ===");
    assert(head != NULL);
    assert(result != NULL);
    test_print_item(result, stdout);

    delete((void *)0x7469);
    puts("=== DELETE LAST NODE ===");
    assert(head != NULL);
    assert(tail == result);
    test_print_all(stdout);
    
    delete((void *)0x1234);
    delete((void *)0x6974);
    puts("=== DELETE ALL NODE ===");
    assert(head == NULL);
    assert(tail == NULL);
    test_print_all(stdout);
}

int main() {
    test_dmem();
}

#endif
