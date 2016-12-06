CFLAGS = -lpaho-mqtt3cs -ljson-c -Wall -g -rdynamic 
all:
	gcc test_msg.c msgs.c LinkList.c mqtt.c -o test_msg $(CFLAGS)

clean:
	rm test_msg
