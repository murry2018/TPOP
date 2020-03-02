// StopWatch: 알고리즘 시간측정을 위한 템플릿 메서드 클래스이다.
class Stopwatch {
    // beforeTask: 매 루프마다 초기화할 작업을 지정한다. 이 부분은
    // 시간 측정에서 빠진다.
    public void beforeTask() { }
    // doTask: 시간을 측정할 작업을 지정한다.
    public void doTask() { }
    // afterTask: 매 루프가 끝날때마다 실행할 작업을 지정한다. 이
    // 부분은 시간 측정에서 빠진다.
    public void afterTask() { }
    // runTest: 작업을 times 만큼 실행한 시간(밀리세컨드)을 측정한다.
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
    // Sorting 시간을 측정하고, 진행률을 보여준다.
    public static void main(String[] args) {
	final int times = 10000; // 테스트 반복 횟수이다.
	Stopwatch testSort = new Stopwatch() {
		final int size = 1000;              // 정렬할 배열의 크기이다.
		Integer[] data = new Integer[size]; // 정렬할 배열이다.
		int cntTask = 0;		    // 지금까지 완료한 테스트 개수이다.
		final int MAXCOL = 120;		    // 100%에 해당하는 진행률 표시 길이이다.
		int curCol = 0;			    // 현재까지 찍힌 진행률 표시 길이이다.
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
		// getTaskRatio: 완료된 task의 비율을 나타냅니다.(0~1)
		private double getTaskRatio()
		{
		    return (double)cntTask/(double)times;
		}
		// getProgressRatio: 진행률이 찍힌 비율을 나타냅니다.(0~1)
		private double getProgressRatio()
		{
		    return (double)curCol/(double)MAXCOL;
		}
		@Override
		public void afterTask()
		{
		    ++cntTask;
		    if (getTaskRatio() > getProgressRatio()) {
			// 진행률을 보여준다(MAXCOL만큼 찍으면 100%)
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
