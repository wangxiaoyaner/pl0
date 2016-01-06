compile:main.o lexer.o parser.o symbtable.o global.o optimization.o new.o baseblock.o
	g++ -Wall -o compile main.o lexer.o parser.o symbtable.o global.o optimization.o new.o baseblock.o 
clean:rm compile *.o
