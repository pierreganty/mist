normal: $(subst .c,.o,$(wildcard *.c)) 
	rm mist2.o

.SUFFIXES = .c .o

.c.o:
	gcc -DTRANSFERT -DVERSION=2 -ggdb -c $< -o $@

clean:
	$(RM) *.o
