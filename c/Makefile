
INCLUDE_PATH = src/include

GPP = g++ -I ${INCLUDE_PATH} -g

driver : src/driver.cpp 
	${GPP} -o $@ $<

%.o : %.cpp
	${GPP} -c -o %@ $<

test : driver
	./driver

.PHONY: test clean
