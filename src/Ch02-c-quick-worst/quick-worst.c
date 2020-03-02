/* 
 * quick-worst.c:
 * qsort �Լ��� �־��� ���̽��� ã�´�.
 */
#include <Windows.h>	/* GetTickCount */
#include <stdio.h>	
#include <stdlib.h>	/* qsort, rand */
#include <string.h>	/* memcpy */

enum __FOR_TEST {
  NMEMB       = 50000,  /* �׽�Ʈ�� �迭�� ũ�� */
  NTEST       = 10000,	/* �׽�Ʈ Ƚ�� */
};

enum __FOR_PRINT {
  ITEM_INLINE = 15,	/* �� �ٿ� ����� ���� ���� */
  MAX_COL     = 120,	/* �ܼ��� �ִ� �� */
};

/* icmp: qsort�� ���� int�� ���Լ� */
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

/* time_to_sort: �־��� �迭�� ���� qsort ���� �ð��� �����Ѵ� */
int time_to_sort(int v[], int nmemb)
{
  DWORD start, end;
  start = GetTickCount();
  qsort(v, nmemb, sizeof(int), icmp);
  end = GetTickCount();
  return (int) (end - start);
}

/* fill_rand: �־��� �迭�� [0, nmemb) ������ ������ ������ ä��� */
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

  int v[NMEMB],		/* ���� ��� */
    origin[NMEMB];	/* ���� �� v�� �纻 */

  /* �ʱ� ���� ���� ���ĵ� ����  */
  for (int i = 0; i < NMEMB; i++)
    v[i] = NMEMB-i;
  memcpy(worst_arr, v, sizeof v);
  worst_time = time_to_sort(v, NMEMB);
  /* �迭�� �����ϰ� ä��� worst case�� ã�´�. */
  int col = 0;	 /* col: ���� ���븦 ���� ���� */
  for (int i = 0; i < NTEST; i++) {
    fill_rand(v, NMEMB);
    memcpy(origin, v, sizeof v);
    int elapsed = time_to_sort(v, NMEMB);
    if (elapsed > worst_time) {
      worst_time = elapsed;
      memcpy(worst_arr, origin, sizeof origin);
    }
    /* ���� ���븦 ǥ���Ѵ�. */
    if (test_ratio(i) > progress_ratio(col)) {
      while (test_ratio(i) > progress_ratio(col)) {
	fputc('=', stderr);
	col++;
      }
    }	
  }
  printf("==== Worst Case ===\n");
  /* ITEM_INLINE ���� ��ŭ ��� ����Ѵ� */
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

