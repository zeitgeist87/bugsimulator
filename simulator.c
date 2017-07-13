/*
 * gc_test.c
 *
 *  Created on: 12.10.2013
 *      Author: andreas
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>

#define BUF_SIZE_SHIFT		12
#define BUF_SIZE (1 << BUF_SIZE_SHIFT)
// Default size in GB
#define TOTAL_SIZE		50
#define NUM_FILES		(TOTAL_SIZE << (30 - BUF_SIZE_SHIFT))

int writeFull(int fd, const void *buf, size_t count) {
	for (;;) {
		ssize_t ret = write(fd, buf, count);
		if (ret >= 0) {
			count -= ret;
			buf += ret;
			if (count == 0)
				return 0;

			// Back off for a bit
			sleep(1);
		} else if (errno == ENOSPC) {
			// Back off for a bit
			sleep(30);
		} else {
			return ret;
		}
	}

	return -1;
}
int writeFile(const char *prefix, unsigned int i) {
	char buf[BUF_SIZE];
	int fd;

	sprintf(buf, "%s/%X/%X/%X", prefix, i / (1000 * 1000), (i / 1000) % 1000, i % 1000);

	fd = open(buf, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1){
		perror("ERROR: opening testfile");
		return -1;
	}

	if (writeFull(fd, buf, sizeof(buf))){
		perror("ERROR: writing testfile");
		close(fd);
		return -1;
	}

	// Success
	close(fd);
	return 0;
}

int createDir(const char *prefix, unsigned int i) {
	char buf[512];

	sprintf(buf, "%s/%X", prefix, i / 1000);

	if (mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST) {
		perror("ERROR: creating directory");
		return -1;
	}

	sprintf(buf, "%s/%X/%X", prefix, i / 1000, i % 1000);

	if (mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST) {
		perror("ERROR: creating directory");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv){
	unsigned int i = 0;

	if (mkdir("workdir", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST) {
		perror("ERROR: creating cold directory");
		return EXIT_FAILURE;
	}

	// Use two layers of directories to get better performance
	for (i = 0; i < (NUM_FILES + 999) / 1000; ++i) {
		if (createDir("workdir", i))
			return EXIT_FAILURE;
	}

	// Write until the file system crashes
	for (;;) {
		for (i = 0; i < NUM_FILES; ++i) {
			if (writeFile("workdir", i))
				return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
