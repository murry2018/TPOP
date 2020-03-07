#if !defined ( GUARD__DEBUG_H )
#define GUARD__DEBUG_H

 #if !defined ( __DEBUG_MODE )

 #define dmalloc malloc
 #define dfree free

 #else
 #include <stdio.h>

 /* dmalloc: 디버그를 위한 할당 함수. 메모리 넘침이 감지되면
  * 에러메시지를 출력하고 종료한다. */
 extern void *dmalloc(size_t);
 /* dfree: 디버그를 위한 해제 함수. dmalloc으로 할당되지 않았거나
  * 메모리가 훼손된 경우 에러메시지를 출력하고 종료한다. */
 extern void dfree(void *);
 /* debug_is_clean: dmalloc으로 할당된 메모리에 메모리 침범이 있는지
  * 확인하고 무결성이 확인되면 1, 그렇지 않으면 0을 반환한다. */
 extern int debug_is_clean(void *addr);
 /* debug_dump: dmalloc으로 할당된 모든 메모리에 대해 무결성을
  * 확인하고 무결하다면 내용을 출력한다. 그렇지 않다면 메모리가
  * 훼손되었다는 메시지를 출력한다. 메모리 값은 16진수와 아스키 문자로
  * 각각 출력되고, 메모리 주소도 16진수로 출력된다. */
 extern void debug_dump(FILE* fout);

 #endif

#endif
