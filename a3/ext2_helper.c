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

void close_image(){
	if (munmap(ext2_image, image.st_size)){
		perror("Updating image");
	}

	if (close(fd)){
		perror("Closing image");
	}
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

  		int next = file_exists(walk, buffer);

  		if (next == -1 || i == steps - 1){
  			index = next;
  			break;
  		}

  		walk += (Inode *) &ext2_image[addr_root + next * INODE_SIZE - INODE_SIZE];

  	}

	return index;
}

int file_exists(Inode *dir, char *filename){
	int index = -1, i;

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){
	  		if (!strncmp(dentry->name, filename, dentry->name_length)){
	  			index = dentry->inode;
	  			break;
	  		}
	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}

	  	if (index != -1)
	  		break;
	  }

  	return index;
}

int mk_file_entry(Inode *dir, char *filename, int type){
	int index = -1, i;
	Dir_entry *dentry = -1;

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}

	  	if (i == 12)
	  		return index;

	  }

	  index = find_free_inode();

	  if (index != -1){
	  	dentry->inode = (uint32_t) index;
		dentry->size = (uint16_t) strlen(filename) + 9;
		dentry->name_length = (char) strlen(filename);
		dentry->type = (char) type;
		strncpy(dentry->name, filename, (int) dentry->name_length + 1);
	  }

  	return index;
}

void rm_file_entry(Inode *dir, char *filename){
	int i = 0;

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){
	  		if (!strncmp(dentry->name, filename, dentry->name_length)){
				dentry->inode = 0;
				dentry->size = 0;
				dentry->name_length = 0;
				dentry->type = 0;
				dentry->name = 0;
	  			return;
	  		}
	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}
	  }
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

void toggle_data_bitmap(int index){

}