
INCLUDE_PATH = include

FILES = \
  src/data.o\
  src/rwlock.o\
	src/jitd_tester.o\
	src/cog_tester.o

HEADERS = $(shell find ${INCLUDE_PATH} -name '*.hpp')

CPP_FLAGS = \
	-std=c++11 

CPP = g++ -I ${INCLUDE_PATH} -g $(CPP_FLAGS)

driver : ${FILES} src/driver.cpp 
	@echo "Building Driver"
	@${CPP} -o $@ $^

%.o : %.cpp
	@echo Compiling $(patsubst src/%,%,$*)
	@${CPP} -c -o $@ $<

build_test : driver
	@for i in test/*.cog; do echo ===== $$i =====; ./driver -c $$i; done

test : build_test

clean : 
	rm -rf *.dSYM
	rm -f driver
	find src -name '*.o' | xargs rm -f

# ,--- Yes, I know this is a horrible hack.
# V    I'm too annoyed with C++ right now to get makedep working.
src/jitd_tester.o : ${HEADERS}
src/cog_tester.o : ${HEADERS}

foo: 
	echo ${HEADERS}

.PHONY: test clean build_test dep
