INC=-I. -I.. -I/usr/local/include
LIB=-L/usr/local/lib
FLAGS=-std=c++14 -g

all: libqicq.dylib qicq_test

clean:
	rm qicq_test libqicq.dylib *.o

hunit.o: hunit.cpp hunit.h
	clang++ $(FLAGS) $(INC) -c $(filter %.cpp,$^)

install: libqicq.dylib qicq.h qicq_adapt.h qicq_fun.h qicq_lambda.h qicq_math.h qicq_sym.h
	cp $(filter %.dylib,$^) /usr/local/lib
	mkdir -p /usr/local/include/qicq
	cp $(filter %.h,$^) /usr/local/include/qicq

libqicq.dylib: qicq.o qicq_fun.o qicq_sym.o
	clang++ -shared $^ -o $@

qicq.o: qicq.cpp qicq.h
	clang++ $(FLAGS) $(INC) -c $(filter %.cpp,$^)

qicq_fun.o: qicq_fun.cpp qicq_fun.h
	clang++ $(FLAGS) $(INC) -c $(filter %.cpp,$^)

qicq_sym.o: qicq_sym.cpp qicq_sym.h 
	clang++ $(FLAGS) $(INC) -c $(filter %.cpp,$^)

qicq_test: qicq_test.cpp qicq.o qicq_fun.o qicq_adapt.h qicq_math.h qicq_sym.o hunit.h hunit.o
	clang++ $(FLAGS) $(INC) -o $@ $(filter %.cpp %.o,$^)
