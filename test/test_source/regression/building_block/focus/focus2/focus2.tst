#!/bin/sh
rm -f input.script
../../../../../executables/forever_cxx/forever >> file &
ls -l file
cat file
read i < file
rm file
../../../../../executables/forever_cxx/forever >> file &
ls -l file
cat file
read j < file
rm file
echo expCreate  -p $i usertime>> input.script
echo expCreate -p $j usertime >> input.script
echo expfocus  >> input.script
echo expfocus -x 1 >> input.script
echo expfocus >> input.script
echo exit >> input.script
openss -batch < input.script
kill $i $j
