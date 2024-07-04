const process = require("process");

if (process.argv.length < 3) {
	print("Please provide the amount of numbers to sum together")
	process.exit(1)
}

let n = Number(process.argv[2])
let sum = 0;
let t1 = Date.now()
for (let i = 0; i < n; i++)	sum += i & 0xff;
let t2 = Date.now()

console.log(`Summing took ~${t2 - t1}ms`);
console.log(`Sum: ${sum}`);
