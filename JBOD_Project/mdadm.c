#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "jbod.h"
#include "mdadm.h"


// creation of bitfield for JBOD_operation. The function jbod_operaton is the device driver function that allows our operating system to directly talk to our computer hardware
// our command must be passed to the bitfield because the command must be in the bitfield format for it to be passed to the op arguement for JBOD operation
// a command can involve itself or it can involve itself andor commands that involve the disk
// the arguements that will be passed to our bitfield are the constants created from typedef enum
uint32_t bitfield(uint32_t command, uint32_t diskid, uint32_t reserved, uint32_t blockid)
{
	

	uint32_t op;
	uint32_t s_command = command & 0x3f;
	uint32_t s_diskid = (diskid << 6) & 0x3c0;
	uint32_t s_blockid = (blockid << 10) & 0xffc00;
	op = s_command | s_diskid | s_blockid;

	return op;
}


int is_mounted = 0;
int is_written = 0;


int mdadm_mount(void) {
  if(is_mounted == 0){
    printf("\n\tnow mounting disk\n\n");
    is_mounted = 1; 
    return is_mounted; 
  }
  else if(is_mounted == 1 ){
  printf("\n\tdisk is already mounted, returning -1 \n\n");
  return -1; 

  }
  return 0;
}

int mdadm_unmount(void) {
  if(is_mounted == 1){
    printf("\n\tnow unmounting system, returning 1\n\n");
    is_mounted = 0; 
    return 1; 
  }

  else if(is_mounted == 0){
    printf("\n\tThe system is already unmounted, returning -1\n\n");
    return -1;

  }
  return 0;
}
  
 
int permission_status = 0; 

int mdadm_write_permission(void){
  if(permission_status == 0){
  printf("\n\ngranting permission to write\n\n");
  permission_status = 1;
  return 1;

  }



 
  return 0;
}


int mdadm_revoke_write_permission(void){
  if(permission_status == 1){
  printf("\n\nrevoking permission to write\n\n");
  permission_status = 0;
  return 1;
  }
	return 0;
}


int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
  if (is_mounted == 0 || read_len >= 1024 || start_addr + read_len > JBOD_TOTAL_DISK_BYTES || (read_buf == NULL && read_len != 0))
  {
    printf("RETURN ERROR\n ");
    return -1;
  }
// why does dividing a specified starting address by the total amount of bytes give us 
// the current disk we are in?
  int current_disk = start_addr / JBOD_DISK_SIZE;

  int current_block = (start_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
  int blockoffset = (start_addr % JBOD_BLOCK_SIZE);

  uint32_t bytes_read = 0;
  uint32_t remaining_bytes = read_len;
  uint32_t bytes_to_read;
// only a specified amount of bytes can be read at a time
//and that specified amount is added to a variable for each iteration. 
  printf("loop is now starting\n\n");
  while (bytes_read < read_len)
  {
    //this array will hold the size of one block 
  // reading is going from a memory address to a source
  
    uint8_t tempbuf[JBOD_BLOCK_SIZE];
    // creation of unsigned 8 bit array that will hold the starting address and size of one block 
     // Temporary buffer to store the result of jbod read

    // current_disk = (start_addr + bytes_read)/JBOD_DISK_SIZE; // Updated current disk
    //		current_block = (((start_addr+bytes_read)%JBOD_DISK_SIZE)/JBOD_BLOCK_SIZE);// Updated current block
    // 
    
    if (jbod_client_operation(bitfield(JBOD_SEEK_TO_DISK, current_disk, 0, 0), NULL) != 0){
      printf("Seek to Disk has failed\n\n"); // Seek to disk
      return -1;}
    if (jbod_client_operation(bitfield(JBOD_SEEK_TO_BLOCK, 0, 0, current_block), NULL) != 0)
      {printf("Seek to block has failed\n\n"); // Seek to blk
      return -1;}
    // if the jbod operation performs the command read bloc, which must first be passed to the bitfield,
      
    if (jbod_client_operation(bitfield(JBOD_READ_BLOCK, 0, 0, 0), tempbuf) != 0){
          printf("Reading has failed\n\n");
            return -1;
            } // Read the current block
      // bytes to read is currently the size of the specific memory addresses we want to read
      //
    bytes_to_read = JBOD_BLOCK_SIZE - blockoffset;
    if (bytes_to_read > remaining_bytes)
    {
      bytes_to_read = remaining_bytes;
    }
  //doing tempbuff + blockoffset will give us the starting address of where we want to read 
    memcpy(read_buf + bytes_read, tempbuf + blockoffset, bytes_to_read);
    bytes_read += bytes_to_read;
    remaining_bytes -= bytes_to_read;
    blockoffset = 0;
    // how to find the next block?
    // increment current block
    // check if current block >= then go to next disk
    current_block++;
    if (current_block >= JBOD_NUM_BLOCKS_PER_DISK)
    {
      current_block = 0;
      current_disk++;
      if (current_disk >= JBOD_NUM_DISKS)
      {
        printf("disk is greater than number of disks");
        break;
      }
    }
  }
  printf("Value of read_len: %d , Value of bytes_read: %d", read_len, bytes_read);

  return read_len;
}


//let's understand and break down the arguements 
//will the mdadm_write need a temperoary buffer?
//why did our read function need a temporary buffer array variable?
int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf) {
  // before this function is called the interger variables is_mounted and  must be set to the interger value
  //0 
  if(is_mounted == 0){
    printf("\n\nthe system is unmounted, cannot perform write operation \n\n");
    return -1;
  }
  if(permission_status == 0){
    printf("\n\n the system does not have permission to perform writing operation on Disks, returning -1 ");
    return -1; 


  }
//
  if(start_addr + write_len > JBOD_TOTAL_DISK_BYTES ){
    printf("\n\n\t starting address is greater than the total amount of bytes within all disks combined \n\n");

    return -1;
  }
// how do we create a conditon for 
//we would pass sizeof(write buffer) to output the 
  if(write_len > 1024){
    return -1;
  }

  if((write_buf == NULL && write_len != 0)){
    return -1;
  }

  printf("valid parameters, now continuing the rest of the code\n\n");

  //assigning variables to the output of
  // we create these va
  //how is JBOD_operation being used within this function? 
  // 700 / 65,536 = 0
  // one full block could not fit into 700
  //how many groups of 65,536(one disk) can fit into 700?
  // 0 so the current disk is 0?
  int current_disk = start_addr / JBOD_DISK_SIZE ; //creating expression to locate which 
                                                  // disk a specified starting adress is within                                        
  int current_block = (start_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
  //distance between starting address and and beginning of block 
  int blockoffset =  (start_addr % JBOD_BLOCK_SIZE);

  uint32_t bytes_read = 0;
  uint32_t remaining_bytes = write_len;
  uint32_t bytes_to_read;

  while (bytes_read < write_len)
  {//this array will hold the size of one block 
  // reading is going from a memory address to a source
  
    uint8_t tempbuf[JBOD_BLOCK_SIZE];
    // creation of unsigned 8 bit array that will hold the starting address and size of one block 
     // Temporary buffer to store the result of jbod read

    // current_disk = (start_addr + bytes_read)/JBOD_DISK_SIZE; // Updated current disk
    //		current_block = (((start_addr+bytes_read)%JBOD_DISK_SIZE)/JBOD_BLOCK_SIZE);// Updated current block
    // 
    if (jbod_client_operation(bitfield(JBOD_SEEK_TO_DISK, current_disk, 0, 0), NULL) != 0) // Seek to disk
      return -1;
    if (jbod_client_operation(bitfield(JBOD_SEEK_TO_BLOCK, 0, 0, current_block), NULL) != 0) // Seek to blk
      return -1;
    // if the jbod operation performs the command read bloc, which must first be passed to the bitfield,
    if (jbod_client_operation(bitfield(JBOD_WRITE_BLOCK, 0, 0, 0), tempbuf) != 0) // Read the current block
      return -1;
      // bytes to read is currently the size of the specific memory addresses we want to read
      //
    bytes_to_read = JBOD_BLOCK_SIZE - blockoffset;
    if (bytes_to_read > remaining_bytes)
    {
      bytes_to_read = remaining_bytes;
    }
    memcpy(tempbuf + blockoffset, write_buf + bytes_read, bytes_to_read);
    bytes_read += bytes_to_read;
    remaining_bytes -= bytes_to_read;
    blockoffset = 0;
    
    current_block++;
    if (current_block >= JBOD_NUM_BLOCKS_PER_DISK)
    {
      current_block = 0;
      current_disk++;
      if (current_disk >= JBOD_NUM_DISKS)
      {
        break;
      }
    }
  }

  return write_len;
}