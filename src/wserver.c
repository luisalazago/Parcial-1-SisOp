#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "request.h"
#include "io_helper.h"

#define NAME "/dory"
#define NUM 3
#define SIZE (NUM * sizeof(int))

char default_root[] = ".";

int share_memory(pid_t pid, time_t time_init, time_t time_end){
	int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
	if(fd < 0)
		return EXIT_FAILURE;
	
	ftruncate(fd, SIZE);

	int *data = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	data[0] = (int)pid; data[1] = (int)time_init; data[2] = (int)time_end;

	munmap(data, SIZE);
	close(fd);
	return EXIT_SUCCESS;
}

int get_memory(){
	int fd = shm_open(NAME, O_RDONLY, 0666);
	if(fd < 0)
		return EXIT_FAILURE;
	
	int *data = (int *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);
	//Codigo para ponerlo en sqllite
	munmap(data, SIZE);
	close(fd);
	shm_unlink(NAME);
	return EXIT_SUCCESS;
}
//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
	pid_t pid_serv_manager = fork(); // Wserver process manager Punto 3)
	if(!pid_serv_manager){
		while(1){
			if(!get_memory())
				printf("get data succesefull\n");
		}
	}

	else{
		int flag = 1;
		while (1) {
			if(flag){

				//Punto 2) crear un proceso que se encargue de una petición http
				time_t time_init = time(NULL);
				pid_t pid_request = fork();
				//Creación de los procesos htpps que llegan
				if(!pid_request){ //procesos hijos
					printf("\nCree un proceso hijo\n");
					struct sockaddr_in client_addr;
					int client_len = sizeof(client_addr);
					int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
					request_handle(conn_fd);
					close_or_die(conn_fd);
					flag = 0;
					printf("Termine el proceso Hijo nro: ");
				}
				//Proceso padre
				else{
					flag = 1;
					waitpid(pid_request, NULL, 0);
					printf(" %d\n\n", pid_request);
					time_t time_end = time(NULL);
					while(share_memory(pid_request, time_init, time_end)){
						printf("Error sharing memory trying again\n\n");
					}
				}
			}
			//Terminate the child process
			else
				break;
		}
		waitpid(pid_serv_manager, NULL, 0);
	} 
    return 0;
}


    


 
