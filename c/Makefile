
INCLUDE_PATH = src/include

FILES = \
	src/jitd/cog.o\
	src/jitd/cog_array.o\
	src/jitd/cog_concat.o\
	src/util/cog_builder.o


GPP = g++ -I ${INCLUDE_PATH} -g

driver : ${FILES} src/driver.cpp 
	${GPP} -o $@ $^

%.o : %.cpp
	${GPP} -c -o $@ $<

build_test : driver
	@for i in test/*.cog; do echo ===== Build Cog: $$i =====; ./driver $$i; done

test : build_test

clean : 
	find src -name '*.o' | xargs echo rm -f

.PHONY: test clean build_test
