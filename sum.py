from sys import argv
from time import time

if len(argv) < 2:
	print("Please provide the amount of numbers to sum together")
	exit(1)

n = int(argv[1])

t1  = time()
sum = 0
for i in range(n):
	sum += i&255
t2 = time()

print(f"Summing took ~{(t2 - t1)*1000}ms")
print(f"Sum: {sum}")
