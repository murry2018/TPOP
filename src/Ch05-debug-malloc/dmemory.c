#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "memlist.h"

const Byte CHECKBYTE = 0xfe;

static int isclean(Dmem *mem);
static char *spaces(int n);
static void printmem(Dmem *mem, void *arg);

/* integerity_check: 메모리 무결성 파괴가 감지되면 메시지 출력 후
 * 프로그램을 종료한다. */
static void integrity_check(char *additional)
{
    if (!every(&isclean)) {     
        map(&printmem, stderr);
        fputs(additional, stderr);
        fputs(" Detected broken memory integrity\n",
              stderr);
        abort();
    }
}

/* dmalloc: 디버그를 위한 할당 함수. 메모리 무결성이 깨지면
 * 에러메시지를 출력하고 종료한다. */
void *dmalloc(size_t n)
{
    const int size = n+2;       /* 검증 바이트를 포함한 크기 */
    Byte *real;
    void *addr = NULL;
    Dmem *mem;
    integrity_check("dmalloc:");
    real = (Byte *)malloc(size);
    if (!real)                  /* 메모리 블록 할당 실패 */
        return NULL;
    real[0] = CHECKBYTE;
    real[size-1] = CHECKBYTE;
    addr = (void *)(&real[1]);
    mem = add(makemem(addr, real, size));
    if (!mem) {                 /* Dmem 객체 할당 실패 */
        free(real);
        return NULL;
    }
    return addr;
}

/* dfree: 디버그를 위한 해제 함수. dmalloc으로 할당되지 않았거나
 * 메모리가 훼손된 경우 에러메시지를 출력하고 종료한다. */
void dfree(void *p)
{
    Dmem *mem;
    Byte *real;
    integrity_check("dfree:");
    if (!p)             /* dfree(NULL)은 아무 작업도 수행하지 않는다*/
        return;
    mem = find(p);
    if (!mem) {              /* dmalloc으로 할당한 메모리가 아닐 때 */
        fputs("dfree: dangling pointer error\n", stderr);
        abort();
    }
    real = mem->real;
    real[0] = real[mem->size-1] = 0x0;
    free(real);
    del(mem);
}

/* dmem_isclean: dmalloc으로 할당된 메모리에 메모리 침범이 있는지
 * 확인하고 무결성이 확인되면 1, 그렇지 않으면 0을 반환한다. */
int dmem_isclean(void *addr)
{
    Dmem *mem;
    mem = find(addr);
    return isclean(mem);
}

/* dmem_dump: dmalloc으로 할당된 모든 메모리에 대해 무결성을 확인하고
 * 내용을 출력한다. 메모리 값은 16진수와 아스키 문자로 각각 출력되고,
 * 메모리 주소도 16진수로 출력된다. */
void dmem_dump(FILE* fout)
{
    map(&printmem, fout);
}

/* isclean: mem 객체에 할당된 메모리의 무결성을 확인한다. 메모리
 * 바깥에 덮어쓴 내용이 감지되거나 mem이 NULL인 경우 0을 반환한다.*/
static int isclean(Dmem *mem)
{
    const int endidx = mem->size - 1;
    return mem != NULL &&
        (mem->real[0] == CHECKBYTE &&
         mem->real[endidx] == CHECKBYTE);
}

/* spaces: n(<128)개의 space가 들은 문자열을 반환한다. */
static char *spaces(int n)
{
    static int before_n = 0;
    static char buf[128] = "";
    if (before_n != n) {
        memset(buf, ' ', n);
        buf[n] = '\0';
    }
    return buf;
}

/* printmem: arg로 주어진 파일스트림으로 메모리 정보를 출력한다. */
static void printmem(Dmem *mem, void *arg)
{
    static char addr[128];
    const int NUM_TOSEP = 8;    /* 몇 바이트씩 출력할지 결정 */
    int i, j, next,
        len,                    /* len: 출력된 포인터값의 길이 */
        size;                   /* size: 할당된 메모리 사이즈 */
    FILE *fout;
    Byte *asbytes;

    assert(mem != NULL);
    sprintf(addr, "%p", mem->addr);
    len = strlen(addr);

    fout = (FILE *)arg;
    fprintf(fout, "%s: is%s clean.\n", addr,
            (isclean(mem) ? "" : " not"));

    asbytes = (Byte *)mem->addr;
    size = mem->size - 2;       /* 사용자에게 주어진 크기 */
    /* NUM_TOSEP바이트씩 잘라 출력한다. */
    for (i = 0; i < size; i = next) {
        fputs(spaces(len+2), fout);
        next = i + NUM_TOSEP;
        if (size < next)
            next = size;
        for (j = i; j < next; j++) /* 16진수 출력 */
            fprintf(fout, "%02x ", asbytes[j]);
        for ( ; j < i + NUM_TOSEP; j++)
            fputs("   ", fout);
        fputs("  ", fout);
        for (j = i; j < next; j++) { /* 아스키 출력 */
            if (isprint(asbytes[j]))
                fputc(asbytes[j], fout);
            else
                fputc('.', fout);
            fputc(' ', fout);
        }
        fputc('\n', fout);
    }
}

#if defined ( __DMEMORY_TEST_MAIN )

/* test_printmem: isclean, spaces을 기반으로 하는 printmem 함수를
 * 테스트한다. */
void test_printmem()
{
    const int size = 24,   /* 시험용 배열 크기 */
        ssize = size-2;    /* 검증 바이트를 제외한 크기 */
    Byte *bytes, *real;
    Dmem *newm;
    int i;
    real = (Byte *) malloc(size);
    if (!real) {
        perror("test_printmem");
        abort();
    }
    real[0] = CHECKBYTE;
    real[size-1] = CHECKBYTE;
    bytes = &real[1];
    strcpy(bytes, "hello, world");
    newm = makemem(bytes, real, size);

    printmem(newm, stdout);

    memset(bytes, 'x', ssize);
    bytes[ssize] = '\0';        /* off-by-one 버그를 만든다. */
    printmem(newm, stdout);
}

/* test_dmalloc: dmalloc 함수를 테스트한다. */
void test_dmalloc()
{
    char const *base = "hello, world";
    int len;
    char *str1, *str2;
    len = strlen(base);
    str1 = (char *) dmalloc(len); /* '\0' 공간을 빼먹는 흔한 실수 */
    if (!str1)
        perror("test_dmalloc");
    strcpy(str1, base);
    str2 = (char *) dmalloc(len);
    assert (1 == 0);       /* 이 전에 반드시 abort가 일어나야 한다. */
}

void test_dfree()
{
    void *p = dmalloc(5);
    dfree(p);
    
    puts("Here we go...");    /* 여기까지 반드시 도달해야 한다. */
    dfree(p);
    assert (1 == 0);       /* 이 전에 반드시 abort가 일어나야 한다. */
}

int main()
{
    test_dfree();
}

#endif
