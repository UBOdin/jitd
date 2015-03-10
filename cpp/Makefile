
INCLUDE_PATH = src/include

FILES = \
	src/cog/array.o\
	src/cog/concat.o\
	src/cog/sortedarray.o\
	src/cog/btree.o\
	src/rewrite/rewrite.o\
	src/rewrite/split_array.o\
	src/rewrite/sort_array.o\
	src/rewrite/recur_to_target.o\
	src/rewrite/pushdown_array.o\
	src/iterator/buffer_iterator.o\
	src/iterator/seq_iterator.o\
	src/iterator/merge_iterator.o\
	src/policy/base.o\
	src/policy/cracker.o\
	src/util/cog_tester.o


GPP = g++ -I ${INCLUDE_PATH} -g

driver : ${FILES} src/driver.cpp 
	@echo "Building Driver"
	@${GPP} -o $@ $^

%.o : %.cpp
	@echo Compiling $(patsubst src/%,%,$*)
	@${GPP} -c -o $@ $<

build_test : driver
	@for i in test/*.cog; do echo ===== $$i =====; ./driver $$i; done

test : build_test

clean : 
	rm -rf *.dSYM
	rm -f driver
	find src -name '*.o' | xargs rm -f

.PHONY: test clean build_test dep
