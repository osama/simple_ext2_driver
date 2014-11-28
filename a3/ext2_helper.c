#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ext2.h>

char *ext2_image;
int fd;

int read_image(char **filename){
	struct stat image;

	if ((fd = open(filename, O_RDWR)) == -1) {
        perror("Opening image");
        return 1;
    }

    fstat(fd, &image);

    ext2_image = mmap(NULL, image.st_size, PROT_WRITE, MAP_SHARED, fd, 0)

    if (ext_image = MAP_FAILED){
    	perror("Mapping image");
    	return 1;
    }

    return 0;
}

