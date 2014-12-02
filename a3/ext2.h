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

/*These functions are used to map the image file into memory
and save the changes when the program has finished running.*/
int read_image(char *filename);
int close_image();

/*These functions relate to the manipulation of files and directories
on the image*/
int traverse_path(char *path);
int file_exists(Inode *dir, char *filename);
int mk_file_entry(Inode *dir, char *filename, char type, int index);
void rm_file_entry(Inode *dir, char *filename);

/*These functions are used to manipulate the appropriate bitmaps and
update counts of free and used inodes and data blocks.*/
void sb_unallocated_count(int block_change, int inode_change);
int find_free_block();
int find_free_inode();
void toggle_data_bitmap(int index);
void toggle_inode_bitmap(int index);

//This struct is used to obtain and modify needed information in the superblock.
typedef struct __attribute__((__packed__)) superblock{
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
} Superblock;

//This struct is used to read and modify block group descriptors.
typedef struct __attribute__((__packed__)) block_group{
    uint32_t addr_block_usage;      //Block address of block usage bitmap
    uint32_t addr_inode_usage;      //Block address of inode usage bitmap
    uint32_t addr_inode_table;      //Starting block address of inode table
    uint16_t unallocated_blocks;    //Number of unallocated blocks in group
    uint16_t unallocated_inodes;    //Number of unallocated inodes in group
    uint16_t dir;                   //Number of directories in group
} Block_group;

//This struct represents information contained within inodes.
typedef struct __attribute__((__packed__)) inode{
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
    uint32_t db[12];

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
} Inode;

//This struct is used to read and traverse directory entries.
typedef struct __attribute__((__packed__)) dir_entry{
    uint32_t inode;
    uint16_t size;
    char name_length;
    char type;
    char name[];
} Dir_entry;