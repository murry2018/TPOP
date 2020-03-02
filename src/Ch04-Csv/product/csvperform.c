#include <time.h>

typedef void (*fn_t)(void);

// elapsed_time: 주어진 작업을 하는데 소요된 밀리세컨드 반환
// before과 after은 NULL로 생략될 수 있음
double elapsed_time(int n, fn_t before, fn_t task, fn_t after)
{
    clock_t start, end, elapsed = 0;
    for (int i = 0; i < n; i++) {
        if (before) (*before)();
        start = clock();
        (*task)();
        end = clock();
        elapsed += end - start;
        if (after) (*after)();
    }
    return (double)elapsed/(CLOCKS_PER_SEC/1000);
}

#include <stdio.h>
#include <stdlib.h>
#include "csv.h"

const char *filepath = "C:\\Users\\User\\Desktop\\mywork\\prpr\\Ch04-Csv\\csvgen\\input.txt";
FILE *in = NULL;

void before_job(void)
{
    in = fopen(filepath, "r");
    if (!in) {
        perror("opening input file");
        exit(1);
    }
}

void task(void)
{
    while (csvgetline(in) != NULL)
        ;
}

void after_job(void)
{
    if (fclose(in) == EOF) {
        perror("closing input file");
        exit(1);
    }
}

int main(void)
{
    double time;
    time = elapsed_time(10, &before_job,
                        &task, &after_job);
    printf("elapsed_time: %.2fms", time);
    
}
