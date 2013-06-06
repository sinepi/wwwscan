default:
	#gcc -I/opt/local/include/curl -O3 -lcurl -lpthread threadpool.c wwwscan.c -o wwwscan
	gcc -Wall -O3 -D_REENTRANT threadpool.c wwwscan.c -o wwwscan -lcurl -lpthread
