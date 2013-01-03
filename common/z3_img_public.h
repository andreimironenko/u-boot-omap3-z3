#ifndef _Z3_IMG_PUBLIC_H
#define _Z3_IMG_PUBLIC_H 

// Updating system throught SD card.
#define SDCARD_BLOCK_SIZE     512

#define MAX_BINS_PER_PACKAGE  12
#define Z3_MAGIC_ID1          (('Z' << 24) | ('3' << 16) | ('-' << 8) | 'C')
#define Z3_MAGIC_ID2          (('E' << 24) | ('N' << 16) | ('T' << 8) | 'A')
#define MAX_IMG_SIZE          (96 * 1024 * 1024)

typedef struct
{
    unsigned int  bytes;
    unsigned int  in_pkg_offset;
    unsigned int  load_address;
    unsigned int  partition_bytes;
    unsigned int  checksum;
    unsigned int  version;
    unsigned char skip_bad_block;
    unsigned char reserved[3];
} bin_hdr;


#define PARTITION_TABLE_SIZE (68) // Round up to multiple of 4 
#define MASTER_HDR_DESC_SIZE (32)
#define RESERVED_BYTES (512 - (MAX_BINS_PER_PACKAGE * sizeof(bin_hdr)) - MASTER_HDR_DESC_SIZE - 16 - PARTITION_TABLE_SIZE)
typedef struct
{
    unsigned int  magic1;
    char          num_of_bins;              // number of binary binsonents in package
    char          ignore_version_check;
    char          reserved[2];
    unsigned int  bindata_start_block;
    bin_hdr       bh[MAX_BINS_PER_PACKAGE]; // header of binary
    char          desc[MASTER_HDR_DESC_SIZE]; // package description
    char          reserved2[RESERVED_BYTES];
    unsigned int  magic2;

    /* Save room for MS-DOS style partition table */
    char          partition_table[PARTITION_TABLE_SIZE];
} master_hdr;

#endif
