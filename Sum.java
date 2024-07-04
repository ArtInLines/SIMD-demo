class Sum {
	static public void main(String[] args) {
		if (args.length < 1) {
			System.out.println("Please provide the amount of numbers to sum together");
			return;
		}
		Integer n = Integer.parseInt(args[0]);
		long t1   = System.nanoTime();
		long sum  = 0;
		for (int i = 0; i < n; i++) sum += i&0xff;
		long t2 = System.nanoTime();
		System.out.printf("Summing took ~%fms\n", ((float)(t2 - t1))/1000000);
		System.out.printf("Sum: %d\n", sum);
	}
}