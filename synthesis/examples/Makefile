
FILES = btree

test: $(FILES)
	@for i in $(FILES); do echo ====== Testing $$i ======; ./$$i; done

$(FILES) : % : %_lib.o cracker.o %.o %_driver.c
	gcc -g -o $@ $(*).o $(*)_lib.o cracker.o $(*)_driver.c

%.c : %.ds ../jitd 
	../jitd $< > $@

cracker.o : cracker.c
	gcc -g -c -o cracker.o cracker.c

%.o : %.c
	gcc -g -c -o $@ $<

clean:
	rm -f $(FILES)
	rm -rf *.{dSYM,o}

.PHONY: test clean
