CC = clang
CFLAGS = -c -Wall -Wextra -ggdb3
LFLAGS = -Wall -Wextra

all: main1 main2 main3 test_all
clean:
	rm -f main1 main2 main3 test_all *.o

main1: main1.o simplify.o logic.o laws.o
	${CC} ${LFLAGS} main1.o simplify.o logic.o laws.o -o main1

main2: main2.o simplify.o logic.o laws.o
	${CC} ${LFLAGS} main2.o simplify.o logic.o laws.o -o main2

main3: main3.o simplify.o logic.o laws.o
	${CC} ${LFLAGS} main3.o simplify.o logic.o laws.o -o main3

simplify.o: simplify.c simplify.h logic.h laws.h
	${CC} ${CFLAGS} simplify.c -o simplify.o

logic.o: logic.c logic.h
	${CC} ${CFLAGS} logic.c -o logic.o

laws.o: laws.c laws.h logic.h
	${CC} ${CFLAGS} laws.c -o laws.o

# For testing

test_all: test_all.o logic.o test_logic.o laws.o test_laws.o
	${CC} ${LFLAGS} test_all.o logic.o test_logic.o laws.o test_laws.o -o test_all

test_logic.o: test_logic.c test_logic.h logic.h laws.h
	${CC} ${CFLAGS} test_logic.c -o test_logic.o

test_laws.o: test_laws.c test_laws.h logic.h laws.h 
	${CC} ${CFLAGS} test_laws.c -o test_laws.o

