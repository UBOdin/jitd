
FILES = btree

test: $(FILES)
	@for i in $(FILES); do echo ====== Testing $$i ======; ./$$i; done

$(FILES) : % : %_lib.o cracker.o adaptive_merge.o %.o %_driver.c
	gcc -g -o $@ $(*).o $(*)_lib.o cracker.o adaptive_merge.o $(*)_driver.c

%.c : %.ds ../jitd 
	../jitd $< > $@

cracker.o : cracker.c
	gcc -g -c -o cracker.o cracker.c

adaptive_merge.o : adaptive_merge.c
	gcc -g -c -o adaptive_merge.o adaptive_merge.c

%.o : %.c
	gcc -g -c -o $@ $<

clean:
	rm -f $(FILES)
	rm -rf *.{dSYM,o}

.PHONY: test clean
