CC = gcc

CFLAGS = -O2 -W -Wall

SOURCE = http.c \
		 log.c 

OBJS = $(addsuffix .o, $(basename $(SOURCE)))

.c.o:
	$(CC) ${CFLAGS} -o $@ -c $<

TARGET = http

$(TARGET):$(OBJS)
	$(CC) ${CFLAGS} $^ -o $@

clean:
	/bin/rm -f *.o $(TARGET)
