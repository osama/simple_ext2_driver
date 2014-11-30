#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;
char *file;
struct stat image;

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

    int dir_addr, index;
	char *temp = argv[2];
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		return 1;
	}

	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	if ((index = file_exists(dir, temp)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		return 1;
	}

	if ((index = mk_file_entry(dir, temp, (char) 0x800), -1) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		return 1;
	}

	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	//TODO: Write Inode properly
	
	int i, read = 0, written = 0;

	for (i = 0; i < 12; i++){
		file->db[i] = find_free_block();

		while (written < BLOCK_SIZE){
			if (read = image.st_size)
				break;

			ext2_image[file->db[i] + written] = file[read];
			read++;
			written++;
		}

		if (read = image.st_size)
			break;

		written = 0;
	}

    return 0;
}