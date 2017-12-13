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
#include <sys/type.h>


#define TX_BUF_SIZE   4000

#define DEFAULT_DEVICE "/dev/i2c_slave"

int main(int argc, char **argv)
{
	char tx_buffer[TX_BUF_SIZE];
	int fd;
	uint8_t data;
	int length;
	int i;

	int opt;
	int mode = 0;
	FILE *usage_file = stderr;
	const char *input = DEFAULT_DEVICE;

	if (optind < argc) {
		input = argv[optind];
	}

	if ((fd = open(input, O_RDWR)) == -1) {
		perror("open i2c device");
		exit(EXIT_FAILURE);
	}

	int random = 0;
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

	close(fd);
	return 0;
}
