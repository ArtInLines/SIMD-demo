@echo off
javac Sum.java > NUL
cl sum_all.c winmm.lib /O1 > NUL

@echo on
java Sum.java %1
java -Djava.compiler=NONE Sum.java %1
node sum.js %1
:: python sum.py %1
sum_all %1
