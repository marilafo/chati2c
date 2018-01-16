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

	if (argc < 2){
		printf("Usage\nsudo ./daemon position\n");
		printf("201 : salon\n202 : bedroom\n203 : kitchen\n204 : bathroom\n");
		printf("205 : wc\n206 : office\n207 : entrance\n");
	}
	position = atoi(argv[1]);
	
	if (position < 201 || position > 207)
		position = 201;	


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
		
		if (change_addr("0x77") == -1)
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
	
			length = read(fd, tx_buffer, 2);
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
			has_addr = 0;
			if (delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
		}
			
		else if (tx_buffer[1] == 231){
			printf("Room not free");
			has_addr = 0;
			if (delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;
		}
		
		else if ( tx_buffer[1] < 3 || tx_buffer[1] > 118){
			printf("Wrong address");
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
	
	int master_listen = 1;
	int temp = 0;
	//Attente des demande du slave, peut etre faire un temps d'attente, ou une sortie
	while (master_listen == 1) {	
		printf("Wait for instructions\n");
				
		length = read(fd, tx_buffer, 2);
		printf("[%d;%d]\n",tx_buffer[0], tx_buffer[1]); 	
		if (tx_buffer[0] == 130){
			if (tx_buffer[1] == 139){
		
				ans_buffer[0]=1;
				ans_buffer[1]=119;
				printf("Je suis la\n");

				write(fd, ans_buffer, 2);
			}
			else if (tx_buffer[1] == 140){
				if (position == 201){
					ans_buffer[0] = 207; 
					ans_buffer[1] = 142;
					ans_buffer[2] = 119;

					write(fd, ans_buffer, 2);

					read(fd, tx_buffer, 2);
					int k = tx_buffer[1];
					int l = 0;
					for ( l = 0; l < k; l++){
						write(fd, tx_buffer, 2);
						printf("Lumière salon :%d\n", tx_buffer[1]);
					}
				}
				else{
					ans_buffer[0] = 149;
					ans_buffer[1] = 149;
					ans_buffer[2] = 119;
					write(fd, ans_buffer, 2);
				}
			}	
			else if (tx_buffer[1] == 141){
				temp = rand()%20;

				ans_buffer[0]=temp;
				ans_buffer[1]=119;
			
				printf("Température : %d\n", temp);
				write(fd, ans_buffer, 2);
			}
			else if (tx_buffer[1] == 142){
				temp = rand()%1;

				ans_buffer[0]=temp;
				ans_buffer[1]=119;

				printf("Lumière : %d\n", temp); 
			
				write(fd, ans_buffer, 2);
			}
			else if (tx_buffer[1] == 143){
				switch(position){
					case 201 : 
						ans_buffer[0] = 5;
						ans_buffer[1] = 119;
						write(fd, ans_buffer, 2);
						
						ans_buffer[0] = 's';
						ans_buffer[1] = 'a';
						ans_buffer[2] = 'l';
						ans_buffer[3] = 'o';
						ans_buffer[4] = 'n';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 5);
						break;
					
					case 202 :
						ans_buffer[0] = 7; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'b';
						ans_buffer[1] = 'e';
						ans_buffer[2] = 'd';
						ans_buffer[3] = 'r';
						ans_buffer[4] = 'o';
						ans_buffer[5] = 'o';
						ans_buffer[6] = 'm';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 7);
						break;
					case 203 :
						ans_buffer[0] = 7; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'k';
						ans_buffer[1] = 'i';
						ans_buffer[2] = 't';
						ans_buffer[3] = 'c';
						ans_buffer[4] = 'h';
						ans_buffer[5] = 'e';
						ans_buffer[6] = 'n';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 7);
						break;
				
					case 204 :
						ans_buffer[0] = 8; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'b';
						ans_buffer[1] = 'a';
						ans_buffer[2] = 't';
						ans_buffer[3] = 'h';
						ans_buffer[4] = 'r';
						ans_buffer[5] = 'o';
						ans_buffer[6] = 'o';
						ans_buffer[7] = 'm';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 8);
						break;

					case 205 :
						ans_buffer[0] = 2; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'w';
						ans_buffer[1] = 'c';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 2);
						break;
					
					case 206 :
						ans_buffer[0] = 6; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'o';
						ans_buffer[1] = 'f';
						ans_buffer[2] = 'f';
						ans_buffer[3] = 'i';
						ans_buffer[4] = 'c';
						ans_buffer[5] = 'e';
						
						printf("Name : %s\n", ans_buffer);
						
						write(fd, ans_buffer, 6);
						break;
				
					case 207 :
						ans_buffer[0] = 8; 
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
					
						ans_buffer[0] = 'e';
						ans_buffer[1] = 'n';
						ans_buffer[2] = 't';
						ans_buffer[3] = 'r';
						ans_buffer[4] = 'a';
						ans_buffer[5] = 'n';
						ans_buffer[6] = 'c';
						ans_buffer[7] = 'e';
						printf("Name : %s\n", ans_buffer);
						write(fd, ans_buffer, 8);
						break;	

					default :

						ans_buffer[0] = 0;
						ans_buffer[1] = 119; 
						write(fd, ans_buffer, 2);
						break;

					
					
						

				}
			}
		}
		else if (tx_buffer[0] == 199){
			printf("Suppresion du modules\n");
			has_addr = 0;
			if(delete_i2c_module(fd) == -1)
				return EXIT_FAILURE;

		}
		
		

		
	}

	return 0;
	
}
