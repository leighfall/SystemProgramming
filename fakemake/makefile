all: f mysort fakemake

clean:
	rm -f core *.o f mysort fakemake

f: f.o f1.o f2.o /home/jplank/cs360/objs/libfdr.a
	gcc -g -o f f.o f1.o f2.o /home/jplank/cs360/objs/libfdr.a

f1.o: f.h f1.c
	gcc -c -g -I/home/jplank/cs360/include f1.c

f2.o: f.h f2.c
	gcc -c -g -I/home/jplank/cs360/include f2.c

f.o: f.c
	gcc -c -g -I/home/jplank/cs360/include f.c

mysort: mysort.o /home/jplank/cs360/objs/libfdr.a
	gcc -g -o mysort mysort.o /home/jplank/cs360/objs/libfdr.a 

mysort.o: mysort.c
	gcc -c -g -I/home/jplank/cs360/include mysort.c

fakemake: fakemake.o
	gcc -g -o fakemake fakemake.o /home/jplank/cs360/objs/libfdr.a

fakemake.o: fakemake.c
	gcc -c -g -I/home/jplank/cs360/include fakemake.c
