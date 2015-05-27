start: test

all: synthesis cpp

test: synthesis
	./synthesis/jitd -j cpp/include/policy/DumbCracker.jitd

cpp: synthesis
	make -C cpp

synthesis:
	make -C synthesis

.PHONY: cpp synthesis test start