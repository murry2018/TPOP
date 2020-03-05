#include <stdio.h>
#include "csv.h"

int main(void)
{
    int i;
    char *line;
    char *seps[] = {". ", ". \"",
                    "\" in ", ", ",
                    ". ", ": ", "."};
    int nseps = (sizeof seps) / (sizeof seps[0]);

    resetsep(FIELD_SEP);
    for (int i = 0; i < nseps; i++) {
        fsep(i, seps[i]);
    }
    while ((line = csvgetline(stdin)) != NULL) {
        printf("line = '%s'\n", line);
        for (i = 0; i < csvnfield(); i++)
            printf("field[%d] = '%s'\n", i, csvfield(i));
    }
    return 0;
}
