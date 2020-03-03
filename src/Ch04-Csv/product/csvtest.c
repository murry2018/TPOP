#include <stdio.h>
#include "csv.h"

int main(void)
{
    int i;
    char *line;

    ssep("&comma;");
    while ((line = csvgetline(stdin)) != NULL) {
        printf("line = '%s'\n", line);
        for (i = 0; i < csvnfield(); i++)
            printf("field[%d] = '%s'\n", i, csvfield(i));
    }
    return 0;
}
