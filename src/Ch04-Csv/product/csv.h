#if !defined( GUARD__CSV_H )
#define GUARD__CSV_H

/* csv.h: csv 라이브러리의 인터페이스 */
#include <stdio.h>

extern char *csvgetline(FILE *f); /* 다음 줄을 읽는다. */
extern char *csvfield(int n);     /* n번째 필드를 리턴한다. */
extern int  csvnfield(void);      /* 필드 개수를 리턴한다. */

#endif
