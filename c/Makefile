

TARGETS = btree
LIB_FILES = cog data
REWRITE_FILES = adaptive_merge cracker
FILES = \
  $(patsubst %, src/lib/%, $(LIB_FILES))\
  $(patsubst %, src/rewrites/%, $(REWRITE_FILES))


CC = gcc -Isrc/include -g

test: $(TARGETS)
	@for i in $(TARGETS); do echo ====== Testing $$i ======; ./$$i; done

$(TARGETS) : % : $(patsubst %, %.o, $(FILES)) src/test/%_test.c
	$(CC) -o $@ $^

%.c : %.ds ../jitd 
	../jitd $< > $@

%.o : %.c
	$(CC) -c -o $@ $<

clean:
	rm -f $(patsubst %, %.o, $(FILES))
	rm -rf *.{dSYM,o}

.PHONY: test clean
