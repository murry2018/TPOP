import java.util.Random;

class Quicksort {
    static Random rgen = new Random();
    // Quicksort.sort: v[left]..v[right]�� ����Ʈ�� ����
    static void sort(Object[] v, int left, int right, Cmp cmp)
    {
	int i, last;

	if (left >= right) // ���� ����. ����
	    return;
	swap(v, left, rand(left, right)); // ���� ���Ҹ� v[left]�� �̵�
	last = left;
	for (i = left+1; i <= right; i++) // ���� �и�
	    if (cmp.cmp(v[i], v[left]) < 0)
		swap(v, ++last, i);
	swap(v, left, last);    // ���� ���� ����
	sort(v, left, last-1, cmp); // ���غ��� ���� ���� ����
	sort(v, last+1, right, cmp); // ���غ��� ū ���� ����
    }
    static void sort(Integer[] v, int left, int right, StaticIcmp cmp)
    {
	int i, last;

	if (left >= right) // ���� ����. ����
	    return;
	iswap(v, left, rand(left, right)); // ���� ���Ҹ� v[left]�� �̵�
	last = left;
	for (i = left+1; i <= right; i++) // ���� �и�
	    if (cmp.cmp(v[i], v[left]) < 0)
		iswap(v, ++last, i);
	iswap(v, left, last);    // ���� ���� ����
	sort(v, left, last-1, cmp); // ���غ��� ���� ���� ����
	sort(v, last+1, right, cmp); // ���غ��� ū ���� ����
    }
    // Quicksort.swap: v[i]�� v[j]�� ��ġ�� �ٲ�
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
    // Quicksort.rand: [left, right] �������� ������ ������ ����
    static int rand(int left, int right)
    {
	return left + Math.abs(rgen.nextInt())%(right-left+1);
    }
    // Quicksort.main: ����Ʈ �޼��� ������ �׽�Ʈ�ϱ� ���� �Լ�
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
