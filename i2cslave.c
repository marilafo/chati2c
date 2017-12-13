#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <curses.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TX_BUF_SIZE   4000

#define DEFAULT_DEVICE "/dev/i2c_slave"

#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)

int insmod_module(char *name){
	int fd; 
	size_t image_size;
	struct stat st;
	void *image;

	fd = open(name, O_RDONLY);
	if (fd == -1){
		perror("open modules");
		return -1;
	}

	fstat(fd, &st);
	image_size = st.st_size;
	image = malloc(image_size);
	read(fd, image, image_size);
	close(fd);

	if(init_module(image, image_size, "") != 0){
		perror("init modules");
		return -1;
	}
	free(image);

	return 0;
}

int rmmod_module(char *name){
	if(delete_module(name, O_NONBLOCK) != 0){
		perror("delete module");
		return -1;
	}
	return 0;
}


int main(int argc, char **argv)
{
	char tx_buffer[TX_BUF_SIZE];
	char ans_buffer[TX_BUF_SIZE];
	int fd;
	uint8_t data;
	int length;
	int i;

	int opt;
	int mode = 0;
	FILE *usage_file = stderr;
	const char *input = DEFAULT_DEVICE;



	
	int position = 1;
	int random = 0;
	int i = 0;
	int has_addr = 0;
	char addr;
	while(has_addr != 1){
	
		//Insertion du module aprés un temps aléatoire
		srand(time(NULL));
		random=rand()%1000000;
		usleep(random+position);

		if(insmod_module("bcm2835_slave_mod.ko")!=0){
			perror("error insmod");
			return EXIT_FAILURE;	
		}

		//Ouverture du fichier du module
		if ((fd = open(input, O_RDWR)) == -1) {
			perror("open i2c device");
			exit(EXIT_FAILURE);
		}


		//Tentative de récupération de l'adresse :
		for (i = 0; i < 3 ; i++){
			length = read(fd, tx_buffer, TX_BUF_SIZE);
			if (i > 0 && tx_buffer[0] == position)
				has_addr = 1;		

			for(i = 0; i < length; i++)
			{
				printf("Data received : %d \n", tx_buffer[i]);
			}

			ans_buffer[0]=position;

			write(fd, tx_buffer, length);
		}
		//Aprés 2 essais on s'enlève et on réétablie la connexion
		if (i == 3){
			close(fd)
			if(rmmod_module("bcm2835_slave_mod")!=0){
				perror("error rmmod");
				return EXIT_FAILURE;
			}
				
		}
		else{
			addr = tx_buffer[1];
			break;
		}
	}

	//Changement d'adresse 
	

	//Attente des demande du slave, peut etre faire un temps d'attente, ou une sortie
	while (1) {	
			length = read(fd, tx_buffer, TX_BUF_SIZE);
			for(i = 0; i < length; i++)
			{
				printf("Data received : %d \n", tx_buffer[i]);
			}
			tx_buffer[0]=1;
			tx_buffer[1]=1;
			
			srand(time(NULL));
			random = rand();

			printf("début pause\n");
			sleep(5);
			printf("fin pause\n");

			length=2;
			write(fd, tx_buffer, length);
	}

	//Suppression du module
	close(fd);
	if(rmmod_module("bcm2835_slave_mod")!=0){
		perror("error rmmod");
		return EXIT_FAILURE;
	}
	
	return 0;
	
}
