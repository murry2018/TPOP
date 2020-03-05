#if !defined ( GUARD__CSVGEN_H )
#define GUARD__CSVGEN_H

#include <stdio.h>

/* Csv 데이터 */
typedef struct Csv Csv;
struct Csv {
    char **rows;                /* rows[i]: Csv 행 */
    char ***fields;             /* fields[i]: i 행의 필드들 */
    int nrows,                  /* nrows: 행의 개수 */
        capa,                   /* capa: rows/fields/nfields 할당 블록 수 */
        *nfields;               /* nfields[i]: i 행의 필드 개수 */
};

/* Csv 한 행 */
typedef struct CsvRow CsvRow;
struct CsvRow {
    char **fields;              /* fields[i]: i번째 필드 */
    int *eachsz;                /* eachsz[i]: i번째 필드의 할당 블록 수 */
    int nfields;                /* nfields: 필드의 개수 */
};

Csv *csv_make();                          /* csv 객체 생성 */
CsvRow *csv_row_make();                   /* csvrow 객체 생성 */
CsvRow *csv_row_from(char *f[], int len); /* 문자열 배열 -> CsvRow 객체 */
int csv_row_append(CsvRow *row, char *f); /* CsvRow에 아이템 추가 */
int csv_row_set(CsvRow *row, int idx, char *f); /* CsvRow의 아이템 변경  */
int csv_add_row(Csv *csv, CsvRow* row); /* Csv에 CsvRow 추가 */
int csv_dump(Csv *csv, FILE *out);      /* csv 출력 */

#endif
