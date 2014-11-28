#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <string.h>

#include "ext2.h"

char *ext2_image;
int fd, root;
struct stat image;

int read_image(char *filename){
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

int close_image(){
	if (munmap(ext2_image, image.st_size)){
		perror("Updating image");
	}

	if (close(fd)){
		perror("Closing image");
	}

	return 0;
}

int traverse_path(char *path){

}

void sb_unallocated_count(int block_change, int inode_change){
	struct superblock sb = ext2_image[BLOCK_SIZE];

	sb.unallocated_blocks += block_change;
	sb.unallocated_inodes += inode_change;
}
