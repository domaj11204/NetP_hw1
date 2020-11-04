all:	t_serv01.c
	gcc -I./lib -g -O2 -D_REENTRANT -w t_serv01.c -o t_serv01 ./lib/libunp.a -lpthread


clean:
		rm t_serv01
