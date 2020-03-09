#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

enum Types {
    FIX_TYPE,                   /* 정수 타입임을 나타낸다. */
    FLT_TYPE,                   /* 부동소수점 숫자임을 나타낸다.  */
    CHR_TYPE,                   /* 문자 타입임을 나타낸다. */
    NTYPES,                     /* 나열된 타입의 수 */
    CHG_NUMT,                   /* FIX->FLT 로의 변경을 지시한다 */
    SKIP,                       /* 생략을 지시한다 */
};

enum Types recent=NTYPES;       /* 이전 타입 */

/* count: 타입별 발생 횟수를 나타낸다. */
unsigned long count[NTYPES] = {0};

/* isterm: c가 문자열 종료 문자인지 검사한다. */
int isterm(char c) {
    return c == '\0' || c == EOF;
}

/* nextnum: 문자열 s 위치부터 시작하는 숫자열의 마지막 다음 위치를
 * 가져온다. 즉, 반환된 포인터 위치에는 숫자가 없다. */
char *numend(char *s)
{
    int i;
    for (i = 0; !isterm(s[i]) ; i++)
        if (!isdigit(s[i]))
            break;
    return &s[i];
}

/* gettype_i: 이전 타입에 관계없이, 문자열 s가 어떤 타입의 값으로
 * 시작하는지 알아낸다.  */
enum Types gettype_i(char *s, char **endptr)
{
    char *end;
    if (isterm(*s))
        return NTYPES;
    if (*s == '.') {
        end = numend(++s);
        if (end != s) {   
            *endptr = end;
            return FLT_TYPE;
        } else {          
            *endptr = s;
            return CHR_TYPE;
        }
    }
    end = numend(s);      
    if (end == s) {       
        *endptr = s+1;
        return CHR_TYPE;
    }
    if (*end == '.') {    
        s = end;
        end = numend(s+1);
        if (end == s+1) {
            *endptr = s;
            return FIX_TYPE;
        } else {
            *endptr = end;
            return FLT_TYPE;
        }
    } else {            
        *endptr = end;
        return FIX_TYPE;
    }
}

/* gettype: 문자열 s가 어떤 타입의 값으로 시작하는지 알아낸다.  */
enum Types gettype(char *s, char **endptr) {
    char *end;
    if (isterm(*s))
        return NTYPES;
    assert(recent != SKIP || recent != CHG_NUMT);
    switch (recent) {
    case CHR_TYPE: /* fall-through */
    case NTYPES:
        return gettype_i(s, endptr);
    case FIX_TYPE:
        if (*s == '.') {
            end = numend(s+1);
            if (end != s+1) {
                *endptr = end;
                return CHG_NUMT;
            } else {
                *endptr = s+1;
                return CHR_TYPE;
            }
        }
        end = numend(s);
        if (end == s) {
            *endptr = s+1;
            return CHR_TYPE;
        }
        if (*end == '.') {
            s = end;
            end = numend(s+1);
            if (end != s+1) {
                *endptr = end;
                return CHG_NUMT;
            } else {
                *endptr = end;
                return SKIP;
            }
        } else {
            *endptr = end;
            return SKIP;
        }
        fputs("gettype: Unreachable condition\n", stderr);
        abort();
    case FLT_TYPE:
        end = numend(s);
        if (end != s) {
            *endptr = end;
            return SKIP;
        } else {
            return gettype_i(s, endptr);
        }
    default:
        fputs("gettype: Unreachable default\n", stderr);
        abort();
    }
}

#define BUFSIZ 10

int main() {
    int i;
    enum Types type;
    char buf[BUFSIZ], *endptr, *s;
    while (fgets(buf, BUFSIZ, stdin) != NULL) {
        s = buf;
        while ((type = gettype(s, &endptr)) != NTYPES) {
            switch (type) {
            case CHR_TYPE: /* fall-through */
            case FLT_TYPE: /* fall-through */
            case FIX_TYPE:
                count[type]++;
                recent = type;
                break;
            case CHG_NUMT:
                count[FIX_TYPE]--;
                count[FLT_TYPE]++;
                recent = FLT_TYPE;
                break;
            case SKIP:
                /* do nothing */
                break;
            default:
                fputs("reading input: Unreachable default\n",
                      stderr);
                abort();
            }
            s = endptr;
        }
    }

    for (i = 0; i < NTYPES; i++) {
        switch (i) {
        case CHR_TYPE:
            s = "char"; break;
        case FIX_TYPE:
            s = "fixnum"; break;
        case FLT_TYPE:
            s = "float"; break;
        default:
            fputs("main: Unreachable default\n",
                  stderr);
            abort();
        }
        printf("%s : %lu\n", s, count[i]);
    }
}
