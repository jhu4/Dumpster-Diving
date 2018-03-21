all: rm dv dump
	mkdir -p .build
	mv *.o .build/

rm: rm.o share.o
	g++ rm.o share.o -o rm

dv: dv.o share.o
	g++ dv.o share.o -o dv

dump: dump.o share.o
	g++ dump.o share.o -o dump

rm.o:
	g++ -std=c++14 rm.c -c 

dv.o:
	g++ -std=c++14 dv.c -c	

dump.o: 
	g++ -std=c++14 dump.c -c

share.o:
	g++ -std=c++14 share.c -c	

clean:
	rm rm dv dump
	rm -rf .build