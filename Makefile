all: rm dv dump test
	mkdir -p .build
	mv *.o .build/

rm: rm.o share.o
	g++ rm.o share.o -o rm

dv: dv.o share.o
	g++ dv.o share.o -o dv

dump: dump.o share.o
	g++ dump.o share.o -o dump

test: test.o share.o
	g++ test.o share.o -o test

rm.o:
	g++ -std=c++14 rm.c -c 

dv.o:
	g++ -std=c++14 dv.c -c	

dump.o: 
	g++ -std=c++14 dump.c -c

share.o:
	g++ -std=c++14 share.c -c	

test.o:
	g++ -std=c++14 test.c -c	

clean:
	rm rm dv dump test
	rm -rf .build