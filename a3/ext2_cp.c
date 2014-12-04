#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <strings.h>
#include <time.h>
#include "ext2.h"

extern char *ext2_image;
extern int addr_root;
extern int debug;
char* finalname;

int main (int argc, char **argv){
	struct stat *image;	

	if (argc != 4){	//Checking if an incorrect number of arguments have been provided.
		printf("usage: ext2_cp virtual_disk local_path absolute_path\n");
		return 1;
	}

	//Opening the local file to be copied as read-only
	int fd;
	char *lfile;

	if ((fd = open(argv[2], O_RDONLY)) == -1) {
        perror("Opening local image");
        return 1;
    }

    //Mapping opened file into memory
    image = malloc(sizeof(struct stat));
    fstat(fd, image);

    lfile = mmap(NULL, image->st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);	//Closing local file's file descriptor

    if (lfile == MAP_FAILED){
    	perror("Mapping local image");
    	close_image();
    	return 1;
    }

    //Reading the disk image into memory
	if (read_image(argv[1])){
		return 1;
	}

    //Finding the target directory by traversing the given path
    int dir_addr, index;
	Inode *dir, *file;

	if ((dir_addr = traverse_path(argv[3])) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Setting directory's inode
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

	if (debug){
		printf(" Directory inode: %d\n", addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE);
	}

	//Checking if the file to be copied already exists, if not, making a new file entry
	if ((index = file_exists(dir, finalname)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	if (debug){
		printf("File does not already exist.\n");
	}

	if ((index = mk_file_entry(dir, finalname, (char) 1, -1)) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Obtaining the file's inode to set information
	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

	//Writing default information for the new inode
	Inode * root = (Inode *) &ext2_image[addr_root];
	file->mode = 0x81b4;
	file->uid = root->uid;
	file->size = (uint32_t) image->st_size;

	file->groupid = root->groupid;
	file->hard_links = 1;
	file->disk_sectors = root->disk_sectors;

	//Setting access and creation times for new directory
	int ftime = (int) time(NULL);
	file->access_time = ftime;
	file->creation_time = ftime;
	file->modification_time = ftime;

	sb_unallocated_count(0, -1);
	
	//Reading data byte-by-byte from the file for all direct file pointers
	int i, read = 0, written = 0;

	for (i = 0; i < 12; i++){
		file->db[i] = find_free_block();

		if (file->db[i] == -1){
			file->db[i] = 0;
			fprintf(stderr, "Out of space on %s\n", argv[1]);
			break;
		}

		sb_unallocated_count(-1, 0);

		//Write to data block until it's full
		while (written <= BLOCK_SIZE){
			if (read == (int) image->st_size)
				break;

			ext2_image[BLOCK_SIZE * file->db[i] + written] = lfile[read];
			read++;
			written++;

			if (debug){
				printf("Read byte %d of %d.\n", read, (int) image->st_size);
			}
		}

		if (read == (int) image->st_size)
			break;

		written = 0;
	}

	//Obtaining singly indirect pointer, if needed
	if (read < (int) image->st_size){
		file->db_singly = find_free_block();
		uint32_t *db = (uint32_t *)  &ext2_image[file->db_singly * BLOCK_SIZE];

		if (file->db_singly == -1){
			file->db_singly = 0;
			fprintf(stderr, "Out of space on %s\n", argv[1]);
		}

		//Allocate another data block and write to it until full
		for (i = 0; i < 256; i++){
			db[i] = find_free_block();

			if (db[i] == -1){
				file->db[i] = 0;
				fprintf(stderr, "Out of space on %s\n", argv[1]);
				break;
			}

			sb_unallocated_count(-1, 0);

			//Write to data block until it's full
			while (written <= BLOCK_SIZE){
				if (read == (int) image->st_size)
					break;

				ext2_image[BLOCK_SIZE *db[i] + written] = lfile[read];
				read++;
				written++;
			}

			if (read == (int) image->st_size)
				break;

			written = 0;
		}
	}

	free(image);
    return 0;
}