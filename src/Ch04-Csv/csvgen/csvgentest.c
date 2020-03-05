#include "csvgen.h"
#include <stdio.h>

/* 다음과 같은 데이터를 만들어 낸다:
Entry Number,Text
1,Test String
2,Test String
3,Test String
 */

int main()
{
    char *header_names[] = {
        "Entry Number", "Text"
    };
    Csv *csv = csv_make();
    CsvRow *header = csv_row_from(header_names, 2);
    csv_add_row(csv, header);
    CsvRow *row = csv_row_make();
    char n[] = "1";
    csv_row_append(row, n);
    csv_row_append(row, "Test String");
    csv_add_row(csv, row);
    for (int i = 2; i <= 3; i++) {
        n[0]++;
        csv_row_set(row, 0, n);
        csv_add_row(csv, row);
    }
    csv_dump(csv, stdout);
}
