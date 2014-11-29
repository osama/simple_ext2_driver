#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;
char *file;

int main (int argc, char **argv){
	if (argc != 4){	//Checking if an incorrect number of arguments have been provided.
		printf("usage: ext2_cp virtual_disk local_path absolute_path\n");
		return 1;
	}

	if (read_image(argv[1])){
		return 1;
	}

	int fd;

	if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("Opening local image");
        return 1;
    }

    fstat(fd, &image);

    file = mmap(NULL, image.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    close(0);

    if (ext_image = MAP_FAILED){
    	perror("Mapping local image");
    	return 1;
    }  

    return 0;
}