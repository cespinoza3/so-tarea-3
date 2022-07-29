
CFLAGS = -I. -lpthread
LIBS += -lm

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

bin/main: main.o Makefile
	@mkdir -p ./bin
	$(CC) -o bin/main main.o $(LIBS)

