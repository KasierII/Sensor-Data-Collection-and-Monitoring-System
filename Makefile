CC = gcc
CFLAGS = -Wall -g

all: collector sensors

collector: collector.c log.c ipc.h log.h
	$(CC) $(CFLAGS) -o collector collector.c log.c

sensors: sensors.c ipc.h
	$(CC) $(CFLAGS) -o sensors sensors.c

clean:
	rm -f collector sensors sensor_log.txt

run: all
	-./collector

.PHONY: clean run all
