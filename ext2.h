#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

#ifndef INODE_SIZE
#define INODE_SIZE 128
#endif

int read_image(char *filename);
int close_image();