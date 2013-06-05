default:
	#gcc -I/opt/local/include/curl -O3 -lcurl -lpthread threadpool.c wwwscan.c -o wwwscan
	gcc -O3 threadpool.c wwwscan.c -o wwwscan -lcurl -lpthread