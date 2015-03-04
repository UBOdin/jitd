
INCLUDE_PATH = src/include

GPP = g++ -I ${INCLUDE_PATH} -g

driver : src/util/cog_builder.o src/driver.cpp 
	${GPP} -o $@ $^

%.o : %.cpp
	${GPP} -c -o $@ $<

test : driver
	./driver

clean : 
	find src -name '*.o' | xargs echo rm -f

.PHONY: test clean
