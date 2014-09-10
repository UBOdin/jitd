
FILES = btree

test: $(FILES)
	@for i in $(FILES); do echo ====== Testing $$i ======; ./$$i; done

$(FILES) : % : %_lib.o %.c %_driver.c
	gcc -g -o $@ $(*)_lib.o $(*)_driver.c

%.c : %.ds 
	../jitd $< > $@

%.o : %.c
	gcc -g -c -o $@ $<

clean:
	rm -f $(patsubst %,%.c,$(FILES))
	rm -f $(FILES)
	rm -rf *.{dSYM,o}
	
	
.PHONY: test clean