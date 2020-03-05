#include "csvgen.h"
#include <stdlib.h>
#include <string.h>

typedef char *String;
typedef String *Fields;

/* strdup_n: src를 복사 생성하여 *dst에 담고, 문자열 길이를
 * 반환한다. */
static int strdup_n(String *dst, String src);
/* strcpy_n: src를 dst에 복사하고, 문자열 길이를 반환한다. */
static int strcpy_n(String dst, String src);

enum {
    INIT_SIZE = 1,
    INCREASE_FAC = 2
};

/* csv 객체 생성 */
Csv *csv_make()
{
    Csv *newp;
    newp = (Csv *) malloc(sizeof(Csv)); /* 할당 */
    if (!newp)                          /* 할당 실패 */
        return NULL;
    newp->rows = (String *) malloc(
        INIT_SIZE * sizeof(newp->rows[0]));
    newp->fields = (Fields *) malloc(
        INIT_SIZE * sizeof(newp->fields[0]));
    newp->nfields = (int *) malloc(
        INIT_SIZE * sizeof(newp->nfields[0]));
    newp->nrows = 0;
    newp->capa = INIT_SIZE;
    return newp;
}

CsvRow *csv_row_make()
{
    CsvRow *newp;
    newp = (CsvRow *) malloc(sizeof(CsvRow));
    if (!newp)
        return NULL;
    newp->fields = NULL;
    newp->eachsz = NULL;
    newp->nfields = 0;
    return newp;
}

/* 문자열 배열 -> CsvRow 객체 */
CsvRow *csv_row_from(Fields f, int nmemb)
{
    CsvRow *newp;
    Fields fields;
    String newf;
    int i, j, len, *sizes;

    /* 할당 */
    newp = (CsvRow *) malloc(sizeof(CsvRow));
    fields = (Fields) malloc(
        nmemb * (sizeof fields[0]));
    sizes = (int *) malloc(
        nmemb * (sizeof sizes[0]));
    if (!newp || !fields || !sizes) { /* 할당 실패 */
        free(newp);
        free(fields);
        free(sizes);
    }
    for (i = 0; i < nmemb; i++) {
        len = strdup_n(&newf, f[i]); /* 할당/복사 */
        if (newf == NULL) {          /* 할당 실패 */
            for (j = 0; j < i; j++)
                free(fields[j]);
            free(newp);
            free(fields);
            free(sizes);
            return NULL;
        }
        fields[i] = newf;
        sizes[i] = len + 1;
    }
    newp->fields = fields;
    newp->eachsz = sizes;
    newp->nfields = nmemb;
    return newp;
}

/* CsvRow에 아이템 추가 */
int csv_row_append(CsvRow* row, String f)
{
    Fields newf;
    String news;
    int nfields, len, *sizes;
    nfields = row->nfields + 1;
    /* 할당/복사 */
    newf = (Fields) realloc(row->fields,
        nfields * (sizeof row->fields[0]));
    sizes = (int *) realloc(row->eachsz,
        nfields * (sizeof row->eachsz[0]));
    if (!newf || !sizes) {       /* 할당 실패 */
        free(newf);
        free(sizes);
        return 0;
    }
    len = strdup_n(&news, f);   /* 할당/복사 */
    if (!news) {                /* 할당 실패 */
        free(newf);
        return 0;
    }
    newf[nfields-1] = news;
    sizes[nfields-1] = len+1;
    row->fields = newf;
    row->eachsz = sizes;
    row->nfields = nfields;
    return 1;
}

/* CsvRow의 아이템 변경  */
int csv_row_set(CsvRow* row, int idx, String f)
{
    int len;
    String news;
    if (idx < 0 || row->nfields <= idx)
        return 0;

    len = strlen(f);
    if (row->eachsz[idx] < len+1) {
        /* 할당/복사 */
        news = (String) realloc(row->fields[idx],
            (len + 1) * sizeof(row->fields[idx][0]));
        if (!news)              /* 할당 실패 */
            return 0;
        row->fields[idx] = news;
        row->eachsz[idx] = len + 1;
    }
    strcpy(row->fields[idx], f);
    return 1;
}

/* Csv에 CsvRow 추가 */
int csv_add_row(Csv *csv, CsvRow *row)
{
    String *rows;
    Fields *fields;
    int newcapa, *nfields;
    if (csv->capa < csv->nrows+1) {
        newcapa = INCREASE_FAC * csv->capa;
        /* 할당/복사 */
        rows = (String *) realloc(csv->rows,
            newcapa * (sizeof rows[0]));
        fields = (Fields *) realloc(csv->fields,
            newcapa * (sizeof fields[0]));
        nfields = (int *)realloc(csv->nfields,
            newcapa * (sizeof nfields[0]));
        if (!rows || !fields || !nfields) { /* 할당 실패 */
            free(rows);
            free(fields);
            free(nfields);
            return 0;
        }
        csv->rows = rows;
        csv->fields = fields;
        csv->nfields = nfields;
        csv->capa = newcapa;
    }
    String newrow;
    Fields newf;
    int i, rowlen = 0;
    for (i = 0; i < row->nfields; i++)
        rowlen += strlen(row->fields[i]) + 1;
    /* 할당 */
    newrow = (String) malloc(
        rowlen * (sizeof newrow[0]));
    newf = (Fields) malloc(
        row->nfields * (sizeof newf[0]));
    if (!newrow || !newf) {    /* 할당 실패 */
        free(newrow);
        free(newf);
        return 0;
    }
    char *p = newrow;
    for (i = 0; i < row->nfields; i++) {
        newf[i] = p;
        /* k: p에서 다음 필드 시작 오프셋 */
        int k = strcpy_n(p, row->fields[i]) + 1;
        p += k;
    }
    csv->rows[csv->nrows] = newrow;
    csv->fields[csv->nrows] = newf;
    csv->nfields[csv->nrows] = row->nfields;
    csv->nrows++;
    return 1;
}

int csv_dump(Csv *csv, FILE *out)
{
    int r, f, res;
    char sep;
    for (r = 0; r < csv->nrows; r++)
        for (f = 0; f < csv->nfields[r]; f++) {
            sep = (f < csv->nfields[r] - 1) ? ',' : '\n';
            res = fprintf(out, "%s%c",
                          csv->fields[r][f], sep);
            if (res <= 0)
                return 0;
        }
    return 1;
}

static int strdup_n(String *dst, String src)
{
    int len;
    String newp;
    len = strlen(src);
    newp = (String) malloc(
        (len + 1) * sizeof(src[0]));
    if (newp)
        strcpy(newp, src);
    *dst = newp;
    return len;
}

static int strcpy_n(String dst, String src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
    return i;
}
