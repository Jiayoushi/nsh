#!/bin/bash

test1="cat -v <infile | grep foo | wc > outfile"
test2="who | wc"
test3="who | fgrep -i .edu > foo"
test4="sort < foo | uniq -c >> bar"
test5="ls"

clear && clear

make -s

if [ $? -ne 0 ];then exit 1; fi

echo ---------------- $test1 -------------------
diff <(echo $test1 | answer) <(echo $test1 | nsh)

echo ---------------- $test2 -------------------
diff <(echo $test2 | answer) <(echo $test2 | nsh)

echo ---------------- $test3 -------------------
diff <(echo $test3 | answer) <(echo $test3 | nsh)

echo ---------------- $test4 -------------------
diff <(echo $test4 | answer) <(echo $test4 | nsh)

echo ---------------- $test5 -------------------
diff <(echo $test5 | answer) <(echo $test5 | nsh)

valgrind -q echo $test1 | nsh > /dev/null
valgrind -q echo $test2 | nsh > /dev/null
valgrind -q echo $test3 | nsh > /dev/null
valgrind -q echo $test4 | nsh > /dev/null
valgrind -q echo $test5 | nsh > /dev/null

make -s clean
