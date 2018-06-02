#!/bin/bash

clear && clear

make -s

if [ $? -ne 0 ];then exit 1; fi

declare -a arr=( "cat -v <infile | grep foo | wc > outfile"
                 "who | wc"
                 "who | fgrep -i .edu > foo"
                 "sort < foo | uniq -c >> bar"
                 "ls"
                 "who <hi | grep world | uniq <for <hi >>sweet | echo > file"
                 "ls <hi -l dir2"
                 "ls -a -b -c -d -e -f -g -h"
                 "<hi ls >for -l -s dir2 -x | <hi >hi grep -l world | ls > out >out"
                 ""
                 "ls |"
                 "who<file|wc>>out|")

function compare() {
  echo ---------------- $@ -------------------
  diff <(echo $@ | answer) <(echo $@ | nsh)
  return $?
}

function memory_test() {
  echo $@ | valgrind --leak-check=full --track-origins=yes nsh > /dev/null
}

for i in "${arr[@]}"
do
  compare $i
  if [[ $? -ne 0 ]]; then make clean && exit 1; fi
done

for i in "${arr[@]}"
do
  memory_test $i
done

exit 0

make -s clean
