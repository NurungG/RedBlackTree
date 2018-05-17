test : example.o rbt.o
	gcc -o test example.o rbt.o -g

example.o : ../rbt.h example.c
	gcc -c example.c -g

rbt.o : ../rbt.h ../rbt.c
	gcc -c ../rbt.c -g

clean :
	rm *.o
