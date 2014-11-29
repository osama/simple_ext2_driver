#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <string.h>

#include "ext2.h"


unsigned char *ext2_image;
int fd, addr_root = -1;
struct stat image;

int read_image(char *filename){
	if ((fd = open(filename, O_RDWR)) == -1) {
        perror("Opening image");
        return 1;
    }

    fstat(fd, &image);

    ext2_image = mmap(NULL, image.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)

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
	int index = -1, taken = 0, steps, i, lookup = root;
	Inode *walk;
	char *buffer = NULL, *tmp = path;

	for (steps=0; tmp[steps]; s[steps]=='/' ? steps++ : *tmp++);
	
	if (addr_root == -1){
		Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*ROOT_BLOCK];
		addr_root = bgr->addr_inode_table + INODE_SIZE;
	}

	walk = (Inode *) &ext2_image[addr_root];

	int next;

	tmp = path
	buffer = strtok (tmp, "/");
  	for (i = 0; i < steps; i++){
  		buffer = strtok (NULL, "/");

  		int dblock = walk->db_0 * BLOCK_SIZE;
  		Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];

  		next = -1;

  		while (!*dentry){
  			if (!strncmp(dentry->name, buffer, dentry->name_length)){
  				next = dentry->inode;
  				break;
  			}
  			
  			dentry += dentry->size;
  		}

  		if (next == -1 || i == steps - 1){
  			index = next;
  			break;
  		}

  		walk += (Inode *) &ext2_image[addr_root + next * INODE_SIZE - INODE_SIZE];

  	}

	return index;
}

int create_file(Inode *dir, int data_blocks){

}

void sb_unallocated_count(int block_change, int inode_change){
	Superblock *sb = (Superblock *) &ext2_image[BLOCK_SIZE];
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];

	sb->unallocated_blocks += block_change;
	sb->unallocated_inodes += inode_change;

	bgr->unallocated_blocks += block_change;
	bgr->unallocated_inodes += inode_change;
}

char *interpret_bitmap(int index){
	char *result = malloc(sizeof(char) * BLOCK_SIZE);
}

int find_free_block(){
	Block_group *bgr = &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr.addr_block_usage;
	int free_index;


	return free_index;
}

int find_free_inode(){

}

void toggle_bitmap(int index){

}