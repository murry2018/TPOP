// StopWatch: �˰��� �ð������� ���� ���ø� �޼��� Ŭ�����̴�.
class Stopwatch {
    // beforeTask: �� �������� �ʱ�ȭ�� �۾��� �����Ѵ�. �� �κ���
    // �ð� �������� ������.
    public void beforeTask() { }
    // doTask: �ð��� ������ �۾��� �����Ѵ�.
    public void doTask() { }
    // afterTask: �� ������ ���������� ������ �۾��� �����Ѵ�. ��
    // �κ��� �ð� �������� ������.
    public void afterTask() { }
    // runTest: �۾��� times ��ŭ ������ �ð�(�и�������)�� �����Ѵ�.
    public final long runTest(int times)
    {
        long elapsed = 0;
        for (int i = 0; i < times; i++) {
            beforeTask();
	    
            long start = System.currentTimeMillis();
            doTask();
            long end = System.currentTimeMillis();
            elapsed += end - start;
	    
            afterTask();
        }
	return elapsed;
    }
    // Sorting �ð��� �����ϰ�, ������� �����ش�.
    public static void main(String[] args) {
	final int times = 10000; // �׽�Ʈ �ݺ� Ƚ���̴�.
	Stopwatch testSort = new Stopwatch() {
		final int size = 1000;              // ������ �迭�� ũ���̴�.
		Integer[] data = new Integer[size]; // ������ �迭�̴�.
		int cntTask = 0;		    // ���ݱ��� �Ϸ��� �׽�Ʈ �����̴�.
		final int MAXCOL = 120;		    // 100%�� �ش��ϴ� ����� ǥ�� �����̴�.
		int curCol = 0;			    // ������� ���� ����� ǥ�� �����̴�.
		@Override
		public void beforeTask()
		{
		    for (int i = 0; i < size; i++)
			data[i] = Quicksort.rand(0, 99);
		}
		@Override
		public void doTask()
		{
		    Quicksort.sort(data, 0, size-1, new Increment());
		}
		// getTaskRatio: �Ϸ�� task�� ������ ��Ÿ���ϴ�.(0~1)
		private double getTaskRatio()
		{
		    return (double)cntTask/(double)times;
		}
		// getProgressRatio: ������� ���� ������ ��Ÿ���ϴ�.(0~1)
		private double getProgressRatio()
		{
		    return (double)curCol/(double)MAXCOL;
		}
		@Override
		public void afterTask()
		{
		    ++cntTask;
		    if (getTaskRatio() > getProgressRatio()) {
			// ������� �����ش�(MAXCOL��ŭ ������ 100%)
			while (getTaskRatio() > getProgressRatio()) {
			    System.out.print('=');
			    curCol++;
			}
		    }
		}
	    };
		
	long elapsed = testSort.runTest(times);
	System.out.printf("\nelapsed time: %d millisecond", elapsed);
    }
}
