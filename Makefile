normal: $(subst .c,.o,$(wildcard *.c)) 
	rm mist2.o

.SUFFIXES = .c .o

.c.o:
	gcc -O3 -DTRANSFERT -DVERSION=2 -c $< -o $@

clean:
	$(RM) *.o
