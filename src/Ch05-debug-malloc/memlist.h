#if !defined ( GUARD__MEMLIST_H )
#define GUARD__MEMLIST_H

typedef unsigned char Byte;

/* Dmem: dmalloc 할당 블록을 관리하는 리스트 노드 */
typedef struct Dmem Dmem;
struct Dmem {
    void *addr;                 /* 사용자에게 주는 메모리 */
    Byte *real;                 /* 실제 메모리 위치 */
    size_t size;                /* 할당된 실제 크기 */
    Dmem *prev, *next;          /* 이전, 다음 노드 */
};

/* makemem: 새 Dmem 객체 생성. */
extern Dmem *makemem(void *addr, Byte *real, size_t size);
/* find: memlist에서 주소가 addr인 Dmem 객체를 찾아내 그 포인터를
 * 반환한다. 실패시 NULL */
extern Dmem *find(void *addr);
/* del: memlist에서 Dmem 객체를 삭제함. 단, 실제 객체(mem->real)는
 * 해제되지 않음. 실패시 0. */
extern int del(Dmem *mem);
/* add: memlist에 Dmem 객체 추가. 실패시 NULL */
extern Dmem *add(Dmem *mem);
/* map: memlist의 원소를 하나씩 순회하며 fn을 적용한다. arg는 fn의 두
 * 번째 인자로 들어간다. */
extern void map(void (*fn)(Dmem *mem, void *arg),
                void *arg);
/* every: memlist의 원소 중 pred의 조건을 만족하지 않는 원소가 하나도
 * 없는지 검사하여 모두 만족하면 1, 아니면 0을 반환한다. */
extern int every(int (*pred)(Dmem *mem));

#endif
