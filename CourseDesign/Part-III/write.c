#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv) {
	// fork();
	int testdev = open("/dev/test", O_RDWR);
	// int dev = open("/dev/test", O_RDWR);
	if (testdev < 0){
		printf("open error: %s\n", strerror(errno));
		exit(-1);
	}	
	printf("testdev = %d\n", testdev);
	char buf[] = "hello world";
	int len = write(testdev, buf, sizeof(buf) - 1);
	printf("write len = %d\n", len);
	close(testdev);
	return 0;
}
