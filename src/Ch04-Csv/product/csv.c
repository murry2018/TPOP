#include "csv.h"
#include <stdlib.h>
#include <string.h>

enum {
    NOMEM = -2,                 /* 메모리 부족을 표시하는 시그널 */
    INIT_SIZE = 1,              /* 메모리블록 초기 할당 크기 */
    INCREASE_FACTOR = 2,        /* 재할당시 증가 배수 */
};

static char *line = NULL;       /* 입력된 문자 데이터 */
static char *sline = NULL;      /* split이 쓸, line의 복사본 */
static int maxline = 0;         /* line[]과 sline[]의 크기 */
static char **field = NULL;     /* 필드를 나타내는 포인터들 */
static int maxfield = 0;        /* field[]의 크기(capacity) */
static int nfield = 0;          /* field[]에 저장된 필드 개수 */
static int is_new = 0;          /* 새로운 데이터가 들어왔는가? */
/* 구분자 관련 */
static enum septype
    _septype = DEFAULT_SEP;     /* 현재 구분자 유형 */
/* 임의 문자열 구분자 */
static char *defaultsep = ",";  /* 기본 구분자 */
static char *fieldsep = NULL;   /* 필드 구분에 쓰일 구분자 */
static int  sepsz  = 0;         /* 구분자 할당크기 */
static int  seplen = 0;         /* 구분자 길이 */
/* 필드별 구분자 지원 */
static char **manysep = NULL;   /* 구분자 목록 */
static int  ms_capa = 0;        /* 구분자 목록 할당 크기 */
static int  ms_nmemb = 0;       /* 구분자 개수 */
static int  *ms_eachsz = NULL;  /* 각각의 구분자 할당 크기 */
static int  *ms_eachlen = NULL; /* 각각의 구분자 길이 */

static void reset(void);
static int endofline(FILE *fin, int c);
static char *advquoted(char *p, char *fieldsep);
static int split(void);
static int splits(void);
static int splitf(void);

/* clearfsep: 필드 구분자를 초기화한다. */
static void clearfsep()
{
    int i;
    for (i = 0; i < ms_capa; i++)
        ms_eachlen[i] = 0;
    ms_nmemb = 0;
}

/* resetsep: 구분자 유형을 변경하고 적절한 초기화를 수행한다. */
void resetsep(enum septype type)
{
    /* 임의 문자열 구분자 유형에 대한 초기화 작업 */
    ssep(defaultsep);
    /* 필드별 구분자 유형에 대한 초기화 작업 */
    clearfsep();
    
    _septype = type;
}

int ssep(char *sep)
{
    int len;
    /* 구분자 유형을 변경한다. */
    if (_septype != DEFAULT_SEP || _septype != STRING_SEP)
        _septype = STRING_SEP;
    
    len = strlen(sep);
    if (sepsz < len+1) {        /* 메모리 할당 */
        char *newp;
        sepsz = len + 1;
        newp = (char *) malloc(
                    (sizeof fieldsep[0]) * sepsz);
        if (!newp)              /* 실패 */
            return 0;
        if (fieldsep)
            free(fieldsep);
        fieldsep = newp;
    }
    strcpy(fieldsep, sep);
    seplen = len;
    return 1;
}

int fsep(int idx, char *sep)
{
    int newsz, newnmemb, newsepsz;
    char **newms, *newf;
    int  *new_eachsz, *new_eachlen;
    if (_septype != FIELD_SEP) {
        clearfsep();
        _septype = FIELD_SEP;
    }
    newnmemb = idx+1;
    if (ms_nmemb < newnmemb)
        ms_nmemb = newnmemb;
    /* manysep의 블록이 부족할 경우 확장 */
    if (ms_capa < newnmemb) {  
        newsz = (ms_capa? ms_capa * INCREASE_FACTOR : newnmemb);
        newms = (char **) realloc (manysep,
                    newsz * (sizeof manysep[0]));
        new_eachsz = (int *) realloc(ms_eachsz,
                          newsz * (sizeof ms_eachsz[0]));
        new_eachlen = (int *) realloc(ms_eachlen,
                           newsz * (sizeof ms_eachlen[0]));
        if (newms == NULL || new_eachsz == NULL || new_eachlen == NULL) {
            free(newms);
            free(new_eachsz);
            free(new_eachlen);
            return 0;
        }
        for (int i = ms_capa; i < newsz; i++) {
            newms[i] = NULL;
            new_eachsz[i] = 0;
            new_eachlen[i] = 0;
        }
        manysep = newms;
        ms_eachsz = new_eachsz;
        ms_eachlen = new_eachlen;
        ms_capa = newsz;
    }
    /* manysep[idx]의 메모리 블록이 부족할 경우 확장 */
    newsepsz = strlen(sep)+1; 
    if (ms_eachsz[idx] < newsepsz) { 
        newsepsz = newsepsz;
        newf = (char *) realloc (manysep[idx],
                    newsepsz * (sizeof manysep[idx][0]));
        if (newf == NULL)
            return 0;
        ms_eachsz[idx] = newsepsz;
        manysep[idx] = newf;
    }
    strcpy(manysep[idx], sep);
    ms_eachlen[idx] = newsepsz-1;
    return 1;
}

/* csvgetline: 한 줄 입력 받음, 필요하면 자동으로 메모리 추가 할당함 */
/* 입력 예시: "LU",86.25,"11/4/1998","2:19PM",+4.0625 */
char *csvgetline(FILE *fin)
{
    int i, c;
    char *newl, *news;
    if (line == NULL) {         /* 첫 호출시 메모리 할당 */
        maxline = maxfield = INIT_SIZE;
        line = (char *) malloc(maxline);
        sline = (char *) malloc(maxline);
        field = (char **) malloc(maxfield*sizeof(field[0]));
        if (line == NULL || sline == NULL || field == NULL) {
            reset();
            return NULL;        /* 메모리 부족 */
        }
    }
    for (i=0;(c=fgetc(fin)) != EOF && !endofline(fin, c); i++) {
        if (i >= maxline-1) {   /* line을 확장한다. */
            maxline *= INCREASE_FACTOR;
            newl = (char *) realloc(line, maxline);
            news = (char *) realloc(sline, maxline);
            line = newl;
            sline = news;
            if (newl == NULL || news == NULL) {
                reset();
                return NULL;    /* 메모리 부족 */
            }
        }
        line[i] = c;
    }
    line[i] = '\0';
    if (c == EOF && i == 0)
        return NULL;
    is_new = 1;
    return line;
}

/* reset: 변수를 초기 상태로 만든다. */
static void reset(void)
{
    free(line);                 /* ANSI C 기준으로 free(NULL) 가능 */
    free(sline);
    free(field);
    line = NULL;
    sline = NULL;
    field = NULL;
    maxline = maxfield = nfield = 0;
}

/* endofline: 검사후 \r, \n, \r\n 또는 EOF를 없앤다 */
static int endofline(FILE *fin, int c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if (c == '\r') {
        c = getc(fin);
        if (c != '\n' && c != EOF)
            ungetc(c, fin);     /* line separator가 \r인 경우 더 읽은걸 도로 집어넣는다 */
    }
    return eol;
}

/* split: 입력 줄을 필드로 구분한다. */
static int split(void)
{
    int ret;
    switch (_septype) {
    case DEFAULT_SEP:
        ssep(defaultsep);
        ret = splits();
        break;
    case STRING_SEP:
        ret = splits();
        break;
    case FIELD_SEP:
        ret = splitf();
        break;
    case REGEX_SEP:
        /* fall-through */
    default:
        ret = 0;
    }
    return ret;
}
    

/* splits: 입력 줄을 임의 조합 문자열을 이용해 필드로 구분한다. */
static int splits(void)
{
    char *p, **newf;
    char *sepp;                 /* 입력 줄 내의 구분자를 가리킴 */
    int sepc;                   /* sepp[0] 임시 저장소 */

    if (!fieldsep)
        ssep(defaultsep);
    nfield = 0;
    if (line[0] == '\0')
        return 0;
    strcpy(sline, line);
    p = sline;

    do {
        if (nfield >= maxfield) {
            maxfield *= INCREASE_FACTOR;
            newf = (char **) realloc(field,
                        maxfield * (sizeof field[0]));
            if (newf == NULL) {
                return NOMEM;
            }
            field = newf;
        }
        if (*p == '"')
            sepp = advquoted(++p, fieldsep); /* 시작하는 따옴표는 넘긴다 */
        else
            sepp = strstr(p, fieldsep);
        sepc = sepp[0];
        sepp[0] = '\0';         /* 필드를 자른다. */
        field[nfield++] = p;
        p = sepp + seplen;
    } while (sepc == fieldsep[0] && strncmp(sepp+1, fieldsep+1, seplen-1) == 0);
    return nfield;
}

/* splitf: 입력 줄을 필드마다 다른 구분자로 파싱한다. */
static int splitf(void)
{
    char *p, **newf;
    char *sepp;                 /* 입력 줄 내의 구분자를 가리킴 */
    int sepc;                   /* sepp[0] 임시 저장소 */
    char *fieldsep;             /* 현재 구분자 */
    int seplen;                 /* 현재 구분자의 길이 */
    
    nfield = 0;
    if (line[0] == '\0')
        return 0;
    strcpy(sline, line);
    p = sline;
    if (maxfield < ms_nmemb+1) {
        maxfield = ms_nmemb+1;
        newf = (char **) realloc(field,
                    maxfield * (sizeof field[0]));
        if (newf == NULL)
            return NOMEM;
        field = newf;
    }
    for (int i = 0; i < ms_nmemb; i++) {
        fieldsep = manysep[i];
        seplen = ms_eachlen[i];
        if (!fieldsep) {
            fieldsep = defaultsep;
            seplen = 1;
        }
        if (*p == '"')
            sepp = advquoted(++p, fieldsep); /* 시작하는 따옴표는 넘긴다 */
        else
            sepp = strstr(p, fieldsep);
        sepc = sepp[0];
        sepp[0] = '\0';         /* 필드를 자른다. */
        field[nfield++] = p;
        p = sepp + seplen;
        if (sepc != fieldsep[0] ||
            strncmp(sepp+1, fieldsep+1, seplen-1) != 0)
            break;
    }
    if (*p != '\0') {
        field[nfield++] = p;
    }
    return nfield;
}

/* advquoted: 따옴표로 둘러싼 필드의 따옴표를 제거하고 다음 구분자를
 * 가리키는 포인터를 리턴한다. */
static char *advquoted(char *p, char *fieldsep)
{
    int i, j;
    /* 코드 해설: 
     * 일반적인 경우, 
     * 따옴표가 연속으로 나오는 경우,
     * 따옴표가 한개만 있는 경우로 구분하여 생각하면 편하다.
     * 명세에 맞지 않는 "abc"def" 와 같은 입력은 제대로 처리할 수 없다. 
     * 명세에 맞지 않지만 "abc"def 와 같은 입력은 제대로 처리된다. */
    for (i = j = 0; p[j] != '\0'; i++, j++) {
        if (p[j] == '"' && p[++j] != '"') {
            /* 다음 구분자나 \0이 나오는 부분까지 복사한다 */
            int k = strstr(p+j, fieldsep) - (p+j);
            memmove(p+i, p+j, k);
            i += k;
            j += k;
            break;
        }
        p[i] = p[j];
    }
    p[i] = '\0';
    return p + j;
}

/* csvfield: n번째 필드에 대한 포인터를 리턴한다. */
char *csvfield(int n)
{
    if (n < 0 || n >= nfield)
        return NULL;
    if (is_new) {
        if (split() == NOMEM) {
            reset();
            return NULL;
        }
        is_new = 0;
    }
    return field[n];
}

/* csvnfield: 필드 개수를 리턴한다. */
int csvnfield(void)
{
    if (is_new) {
        if (split() == NOMEM) {
            reset();
            return 0;
        }
        is_new = 0;
    }
    return nfield;
}
