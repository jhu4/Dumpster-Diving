To build my code, do:
>>  make 

To clean up the executables, do:
>> make clean


My implementation basically are in rm.c dv.c dump.c and share.c
share.c contains the useful funtion I made
rm.c dv.c dump.c basically are just a int main() with some logics inside.


test.c and test.sh are for the experiment, to reproduce the experiment:
1. Make sure you have ~/Documents and ./trashcan folder
2. Make sure the ./ folder is on a different partition than ~/Documents
then
>> export DUMPSTER=trashcan/
>> ./test.sh
wait for probably a min then you can see the result

The report is called experiment.pdf