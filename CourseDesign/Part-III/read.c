#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv) {
	// fork();
	int testdev = open("/dev/test", O_RDONLY);
	// int dev = open("/dev/test", O_RDWR);
	if (testdev < 0){
		printf("open error: %s\n", strerror(errno));
		exit(-1);
	}	
	printf("testdev = %d\n", testdev);
	char buf[20] = {0};
	int len = read(testdev, buf, sizeof(buf) - 1);
	printf("read len = %d\n", len);
	printf("read: %s\n", buf);
	close(testdev);
	return 0;
}
