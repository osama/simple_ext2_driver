#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

#ifndef INODE_SIZE
#define INODE_SIZE 128
#endif

#ifndef ROOT_BLOCK
#define ROOT_BLOCK 2
#endif

#include <stdint.h>

int read_image(char *filename);
int close_image();
int traverse_path(char *path);

void sb_unallocated_count(int block_change, int inode_change);

struct superblock{
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t reserved_blocks;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t superblock_addr;

    uint32_t block_size_log;
    uint32_t fragment_size_log;

    uint32_t blocks_in_group;
    uint32_t fragments_in_group;
    uint32_t inodes_in_group;

    uint32_t mount_time;
    uint32_t write_time;
    uint16_t mounts_since_check;

    //Rest of superblock is not relevant to this Assignment
};

struct block_group{
    uint32_t addr_block_usage;      //Block address of block usage bitmap
    uint32_t addr_inode_usage;      //Block address of inode usage bitmap
    uint32_t addr_inode_table;      //Starting block address of inode table
    uint16_t unallocated_blocks;    //Number of unallocated blocks in group
    uint16_t unallocated_inodes;    //Number of unallocated inodes in group
    uint16_t dir;                   //Number of directories in group
};

struct inode{
    uint16_t mode;                  //Type and permissions
    uint16_t uid;                   //User ID
    uint32_t size;                  //Size in bytes
    uint32_t access_time;           
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t deletion_time;
    uint16_t groupid;
    uint16_t hard_links;            //Count of hard links
    uint32_t disk_sectors;          //Count of sectors
    uint32_t flags;                 //Flags
    uint32_t osval;

    //Direct block pointers
    uint32_t db_0;
    uint32_t db_1;
    uint32_t db_2;
    uint32_t db_3;
    uint32_t db_4;
    uint32_t db_5;
    uint32_t db_6;
    uint32_t db_7;
    uint32_t db_8;
    uint32_t db_9;
    uint32_t db_10;
    uint32_t db_11;

    //Indirect block pointers
    uint32_t db_singly;
    uint32_t db_doubly;
    uint32_t db_triply;

    //Additional information
    uint32_t gen;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t fragment_addr;
    uint32_t osval2;
};

struct dir{
    uint32_t inode;
    uint16_t size;
    char name_length;
    char type;
    char *name;
};