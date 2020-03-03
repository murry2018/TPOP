#if !defined( GUARD__CSV_H )
#define GUARD__CSV_H

/* csv.h: csv 라이브러리의 인터페이스 */
#include <stdio.h>

enum septype {
    DEFAULT_SEP,                /* 기본 구분자 유형 */
    STRING_SEP,                 /* 임의 문자열 구분자 유형 */
    FIELD_SEP,                  /* 필드별 구분자 유형 */
    REGEX_SEP,                  /* 정규식 구분자: 미구현. */
};

extern void resetsep(enum septype type); /* 설정된 구분자 초기화 */
extern int ssep(char *sep);              /* 임의 문자열 구분자 지정 */
extern int fsep(int idx, char *sep);     /* idx번째 필드 구분자 지정 */

extern char *csvgetline(FILE *f); /* 다음 줄을 읽는다. */
extern char *csvfield(int n);     /* n번째 필드를 리턴한다. */
extern int  csvnfield(void);      /* 필드 개수를 리턴한다. */

#endif
