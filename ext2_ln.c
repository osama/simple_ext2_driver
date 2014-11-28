#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;

int main (int argc, char **argv){
	if (argc != 4){	
		printf("usage: ext2_ln virtual_disk path1 path2\n");
		return 1;
	}

	if (read_image(argv[1])){
		return 1;
	}


}