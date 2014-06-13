target: sensor_server mcontrol mserver multiserver

sensor_server: sensor_server.o
	gcc -o sensor_server sensor_server.o /usr/local/lib/libmeter.a /usr/local/lib/libsupport.a -lpthread -lm

#raplserver: raplserver.o
#	gcc -o raplserver raplserver.o /usr/local/lib/libmeter.a /usr/local/lib/libsupport.a -lpthread -lm

mcontrol: mcontrol.o
	gcc -o mcontrol mcontrol.o /usr/local/lib/libmeter.a /usr/local/lib/libsupport.a

mserver: mserver.o
	gcc -o mserver mserver.o /usr/local/lib/libmeter.a /usr/local/lib/libsupport.a -lpthread

multiserver: multiserver.o
	gcc -g -o multiserver_13 multiserver.o /usr/local/lib/libmeter.a /usr/local/lib/libsupport.a -lpthread

clean:
	rm $(target) *.o

