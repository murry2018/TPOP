/* 
 * quick-worst.c:
 * qsort 함수의 최악의 케이스를 찾는다.
 */
#include <Windows.h>	/* GetTickCount */
#include <stdio.h>	
#include <stdlib.h>	/* qsort, rand */
#include <string.h>	/* memcpy */

enum __FOR_TEST {
  NMEMB       = 50000,  /* 테스트할 배열의 크기 */
  NTEST       = 10000,	/* 테스트 횟수 */
};

enum __FOR_PRINT {
  ITEM_INLINE = 15,	/* 한 줄에 출력할 원소 개수 */
  MAX_COL     = 120,	/* 콘솔의 최대 열 */
};

/* icmp: qsort에 쓰일 int형 비교함수 */
int icmp(const void *p1, const void *p2)
{
  int v1, v2;
  v1 = *(int *)p1;
  v2 = *(int *)p2;
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return 0;
}

/* time_to_sort: 주어진 배열에 대해 qsort 수행 시간을 측정한다 */
int time_to_sort(int v[], int nmemb)
{
  DWORD start, end;
  start = GetTickCount();
  qsort(v, nmemb, sizeof(int), icmp);
  end = GetTickCount();
  return (int) (end - start);
}

/* fill_rand: 주어진 배열을 [0, nmemb) 범위의 임의의 값으로 채운다 */
void fill_rand(int v[], int nmemb)
{
  for (int i = 0; i < nmemb; i++)
    v[i] = rand() % nmemb;
}

double test_ratio(int cur_test) {
  return (double)cur_test / (double)NTEST;
}

double progress_ratio(int cur_col) {
  return (double)cur_col / (double)MAX_COL;
}

int main()
{
  int worst_arr[NMEMB];
  int worst_time = -1;

  int v[NMEMB],		/* 정렬 대상 */
    origin[NMEMB];	/* 정렬 전 v의 사본 */

  /* 초기 값은 역순 정렬된 상태  */
  for (int i = 0; i < NMEMB; i++)
    v[i] = NMEMB-i;
  memcpy(worst_arr, v, sizeof v);
  worst_time = time_to_sort(v, NMEMB);
  /* 배열을 랜덤하게 채우며 worst case를 찾는다. */
  int col = 0;	 /* col: 진행 막대를 찍은 개수 */
  for (int i = 0; i < NTEST; i++) {
    fill_rand(v, NMEMB);
    memcpy(origin, v, sizeof v);
    int elapsed = time_to_sort(v, NMEMB);
    if (elapsed > worst_time) {
      worst_time = elapsed;
      memcpy(worst_arr, origin, sizeof origin);
    }
    /* 진행 막대를 표시한다. */
    if (test_ratio(i) > progress_ratio(col)) {
      while (test_ratio(i) > progress_ratio(col)) {
	fputc('=', stderr);
	col++;
      }
    }	
  }
  printf("==== Worst Case ===\n");
  /* ITEM_INLINE 개수 만큼 끊어서 출력한다 */
  for (int i = 0; i < NMEMB; i+=ITEM_INLINE) {
    for (int j = 0; j < ITEM_INLINE; j++) {
      if (i+j < NMEMB)
	printf("%d, ", worst_arr[i+j]);
      else
	break;
    }
    printf("\n");
  }
  printf("elapsed time: %d ms\n", worst_time);
  return 0;
}

