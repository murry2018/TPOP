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

/* dmalloc: 디버그를 위한 할당 함수. 메모리 무결성이 깨지면
 * 에러메시지를 출력하고 종료한다. */
void *dmalloc(size_t n)
{
    const int size = n+2;       /* 검증 바이트를 포함한 크기 */
    Byte *real;
    void *addr = NULL;
    Dmem *mem;
    if (!every(&isclean)) {     /* 메모리 무결성 파괴 감지 */
        map(&printmem, stderr);
        fputs("Detected broken memory integrity", stderr);
        abort();
    }
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
    sprintf(addr, "0x%x", mem->addr);
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

int main()
{
    test_printmem();
}

#endif
