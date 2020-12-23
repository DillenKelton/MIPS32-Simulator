make: main.o simulator.o decode.o
	g++ -o simulator main.o simulator.o decode.o

main.o: main.cpp simulator.h
	g++ -c main.cpp

simulator.o: simulator.cpp simulator.h
	g++ -c simulator.cpp

decode.o: decode.cpp simulator.h
	g++ -c decode.cpp