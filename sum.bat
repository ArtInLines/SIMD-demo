@echo off
javac Sum.java > NUL
cl sum_all.c winmm.lib > NUL

@echo on
java Sum.java %1
java -Djava.compiler=NONE Sum.java %1
node sum.js %1
py sum.py %1
sum_all %1
