import java.util.Random;

class Quicksort {
    static Random rgen = new Random();
    // Quicksort.sort: v[left]..v[right]를 퀵소트로 정렬
    static void sort(Object[] v, int left, int right, Cmp cmp)
    {
	int i, last;

	if (left >= right) // 할일 없음. 종료
	    return;
	swap(v, left, rand(left, right)); // 기준 원소를 v[left]로 이동
	last = left;
	for (i = left+1; i <= right; i++) // 집단 분리
	    if (cmp.cmp(v[i], v[left]) < 0)
		swap(v, ++last, i);
	swap(v, left, last);    // 기준 원소 복원
	sort(v, left, last-1, cmp); // 기준보다 작은 집단 정렬
	sort(v, last+1, right, cmp); // 기준보다 큰 집단 정렬
    }
    static void sort(Integer[] v, int left, int right, StaticIcmp cmp)
    {
	int i, last;

	if (left >= right) // 할일 없음. 종료
	    return;
	iswap(v, left, rand(left, right)); // 기준 원소를 v[left]로 이동
	last = left;
	for (i = left+1; i <= right; i++) // 집단 분리
	    if (cmp.cmp(v[i], v[left]) < 0)
		iswap(v, ++last, i);
	iswap(v, left, last);    // 기준 원소 복원
	sort(v, left, last-1, cmp); // 기준보다 작은 집단 정렬
	sort(v, last+1, right, cmp); // 기준보다 큰 집단 정렬
    }
    // Quicksort.swap: v[i]와 v[j]의 위치를 바꿈
    static void swap(Object v[], int i, int j)
    {
	Object temp;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
    }
    static void iswap(Integer v[], int i, int j)
    {
	Integer temp;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
    }
    // Quicksort.rand: [left, right] 범위에서 임의의 정수를 리턴
    static int rand(int left, int right)
    {
	return left + Math.abs(rgen.nextInt())%(right-left+1);
    }
    // Quicksort.main: 퀵소트 메서드 구현을 테스트하기 위한 함수
    public static void main(String[] args)
    {
	int size = 8;
	Integer arr[] = new Integer[size];
	System.out.println("===BEFORE===");
	for (int i = 0; i < size; i++) {
	    arr[i] = rand(0, 99);
	    System.out.print(arr[i] + " ");
	}
	System.out.println("");
	sort(arr, 0, size-1, new Icmp());
	System.out.println("====AFTER====");
	for (int i = 0; i < size; i++) {
	    System.out.print(arr[i] + " ");	    
	}
	System.out.println("");
    }
}
