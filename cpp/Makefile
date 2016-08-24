
INCLUDE_PATH = include

FILES = \
  src/data.o\
  src/rwlock.o\
	src/jitd_tester.o\
	src/cog_tester.o

HEADERS = $(shell find ${INCLUDE_PATH} -name '*.hpp') \
          $(patsubst %.jitd, %.hpp, $(shell find ${INCLUDE_PATH} -name '*.jitd'))

ifeq ($(shell uname), Darwin)
CPP_FLAGS = \
  -std=c++11  
endif

CPP = g++ -I ${INCLUDE_PATH} -g $(CPP_FLAGS)

all: driver workload_gen

driver : ${FILES} src/driver.cpp 
	@echo "Building Driver"
	@${CPP} -o $@ $^

workload_gen : src/workload.cpp
	@echo "Building Workload Generator"
	@${CPP} -o $@ $^

workload/%.jitd : workload_gen
	@if [ ! -d workload ] ; then mkdir workload; fi
	@echo "Building Workload #" $$(basename $@ .jitd)
	@./workload_gen > $@
	@du -h $@

workload : $(patsubst %,workload/%.jitd, 1)

%.o : %.cpp
	@echo Compiling $(patsubst src/%,%,$*)
	@${CPP} -c -o $@ $<

build_test : driver
	@for i in test/*.cog; do echo ===== $$i =====; ./driver -c $$i; done

test : build_test

%.hpp : %.jitd ../synthesis/jitd
	@echo "Rendering" $(patsubst %.jitd,%,$<)
	@../synthesis/jitd $< > $@

clean : 
	rm -rf *.dSYM
	rm -f driver
	find src -name '*.o' | xargs rm -f

# ,--- Yes, I know this is a horrible hack.
# V    I'm too annoyed with C++ right now to get makedep working.
src/jitd_tester.o : ${HEADERS}
src/cog_tester.o : ${HEADERS}

.PHONY: test clean build_test dep workload jitd
