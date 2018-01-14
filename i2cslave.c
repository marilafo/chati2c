#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define TX_BUF_SIZE   4000

#define DEFAULT_DEVICE "/dev/i2c_slave"
#define DEFAULT_ADRESS "/sys/kernel/i2c_slave_dir/address_file" 

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

int change_addr(char *s){
	//Changement d'adresse 
	FILE *addr_file = fopen(DEFAULT_ADRESS,"w+");
	if (addr_file == NULL){
		perror("opening address file");
		return -1;
	}
	
	fputs(s,addr_file);

	fclose(addr_file);
}

int delete_i2c_module(int fd){
	close(fd);
	if(rmmod_module("bcm2835_slave_mod")!=0){
		perror("error rmmod");
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

	int opt;
	int mode = 0;
	FILE *usage_file = stderr;
	const char *input = DEFAULT_DEVICE;
	
	//Position : indique chambre, salon, etc .. Mettre dans fichier de config 
	int position = 202;
	int alea;
	int random = 0;
	int i = 0;
	int j;
	int has_addr = 0;
	char addr;
	while(has_addr != 1){
	
		//Insertion du module aprés un temps aléatoire
		srand(time(NULL));
		random=rand()%1000000;
		usleep(random+position);
		printf("Insertion du modules\n");
		

		if(insmod_module("bcm2835_slave_mod.ko")!=0){
			perror("error insmod");
			return EXIT_FAILURE;	
		}
		
		if (change_addr("0x42") == -1)
			return EXIT_FAILURE;

		//Ouverture du fichier du module
		printf("Ouverture i2c\n");
		if ((fd = open(input, O_RDWR)) == -1) {
			perror("open i2c device");
			exit(EXIT_FAILURE);
		}


		alea = rand()%100;
	

		//Tentative de récupération de l'adresse :

		for (i = 0; i < 3 ; i++){
			printf("boucle : %d\n", i);
			length = read(fd, tx_buffer, TX_BUF_SIZE);
	
			printf("Buffer : [%d,%d]\n",tx_buffer[0], tx_buffer[1]);
		
			int test_question=0;	
			for(j = 0; j < length; j++)
			{
				if (test_question == 1)
					continue;
				if (tx_buffer[j] == 200){
					test_question = 1;
					printf("Coucou\n");
					ans_buffer[0]=alea;
					ans_buffer[1]=position;
					ans_buffer[2]=119;

					write(fd, ans_buffer, 3);
				}

			}
			if (test_question == 1)	
				continue;	
			
			if (tx_buffer[0] == alea){
				has_addr = 1;
				break;		
			}


				
		}
		//Aprés 2 essais on s'enlève et on réétablie la connexion
		//Test de l'addresse passé par le master
		if (i == 3){
			printf("Suppression du modules\n");
			
			if (delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
			
				
		}
		else if (tx_buffer[1] == 230){
			printf("Room not valid");
			if (delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
			return -1;
		}
			
		else if (tx_buffer[1] == 231){
			printf("Room not free");
			if (delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
			return -1;
		}
		
		else if ( tx_buffer[1] < 3 || tx_buffer[1] > 118){
			printf("Suppresion du modules\n");
			has_addr = 0;
			if(delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
		}
		
		else{
			printf("Changement d'adresse");
			addr = tx_buffer[1];
			ans_buffer[0] = alea;
			ans_buffer[1] = 120;
			ans_buffer[1] = 119;
			//ACK
			//write(fd, ans_buffer, 3);

			printf("Address is : Ox%x\n", addr);
			break;
		}
	}
	printf("endi\n");

	//Changement d'adresse 
	char addr_format[4];
	sprintf(addr_format, "0x%x", addr);
	printf("chaine : %s\n", addr_format);

	
	if (change_addr(addr_format) == -1)
		return EXIT_FAILURE;
	

	//Attente des demande du slave, peut etre faire un temps d'attente, ou une sortie
	/*while (1) {	
			length = read(fd, tx_buffer, TX_BUF_SIZE);
			for(i = 0; i < length; i++)
			{
				printf("Data received : %d \n", tx_buffer[i]);
			}
			tx_buffer[0]=1;
			tx_buffer[1]=1;
			
			srand(time(NULL));
			random = rand();
			
			ans_buffer[0]=position;
			ans_buffer[1]=position;

			length=2;
			write(fd, tx_buffer, length);

			//TODO planifier une sortie
	}

	//Suppression du module
	*/
	return 0;
	
}
