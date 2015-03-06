
INCLUDE_PATH = src/include

FILES = \
	src/jitd/cog_array.o\
	src/jitd/cog_concat.o\
	src/jitd/cog_sortedarray.o\
	src/jitd/cog_btree.o\
	src/rewrite/rewrite.o\
	src/rewrite/split_array.o\
	src/rewrite/sort_array.o\
	src/iterator/buffer_iterator.o\
	src/iterator/seq_iterator.o\
	src/iterator/merge_iterator.o\
	src/util/cog_tester.o


GPP = g++ -I ${INCLUDE_PATH} -g

driver : ${FILES} src/driver.cpp 
	${GPP} -o $@ $^

%.o : %.cpp
	${GPP} -c -o $@ $<

build_test : driver
	@for i in test/*.cog; do echo ===== Build Cog: $$i =====; ./driver $$i; done

test : build_test

clean : 
	find src -name '*.o' | xargs rm -f

.PHONY: test clean build_test dep
