#!/bin/bash

test1="cat -v <infile | grep foo | wc > outfile"
test2="who | wc"
test3="who | fgrep -i .edu > foo"
test4="sort < foo | uniq -c >> bar"
test5="ls"
test6="who <hi | grep world | uniq <for <hi >>sweet | echo > file"
test7="ls <hi -l dir2"
test8="ls -a -b -c -d -e -f -g -h"
test9="<hi ls >for -l -s dir2 -x | <hi >hi grep -l world | ls > out >out"

clear && clear

make -s

if [ $? -ne 0 ];then exit 1; fi

function compare() {
 echo ---------------- $@ -------------------
 diff <(echo $@ | answer) <(echo $@ | nsh)
}

function memory_test() {
 echo $@ | valgrind --leak-check=full --track-origins=yes nsh > /dev/null
}

#for (( i=1; i<=8; i++ ))
#do
#  str="test$i"
#done


compare $test1 
compare $test2
compare $test3
compare $test4
compare $test5
compare $test6
compare $test7
compare $test8
compare $test9

memory_test $test1
memory_test $test2
memory_test $test3
memory_test $test4
memory_test $test5
memory_test $test6
memory_test $test7
memory_test $test8
memory_test $test9

make -s clean
