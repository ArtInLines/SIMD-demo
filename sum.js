const process = require("process");

if (process.argv.length < 3) {
	print("Please provide the amount of numbers to sum together")
	process.exit(1)
}

let n = Number(process.argv[2])
let sum;
let t1 = Date.now()
if (n > 100000000) {
	sum = BigInt(0);
	for (let i = 0; i < n; i++) sum += BigInt(i)
} else {
	 sum = 0;
	for (let i = 0; i < n; i++) sum += i
}
let t2 = Date.now()

console.log(`Summing took ~${t2 - t1}ms`);
console.log(`Sum: ${sum}`);
