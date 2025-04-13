#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/* the client socket descriptor for the connection to the server */
//number that is assigned to a successful socket connection
//socket connection can only be identified by its socket descriptor. 
int cli_sd = -1;

/* attempts to read n (len) bytes from fd; returns true on success and false on failure. 
It may need to call the system call "read" multiple times to reach the given size len. 
*/
//can only read a certain amount each time?
static bool nread(int fd, int len, uint8_t *buf) {
  //reading from given buffer 
  int num_read = read(fd,buf,len);
  //the amount read is stored in total
  int total_read = num_read; 
  //while 
  while(num_read < len){

    read(fd,buf + total_read,len);
    total_read += num_read;
    printf("\tsucessfully read from server\n");
  }

  return true; 
}

/* attempts to write n bytes to fd; returns true on success and false on failure 
It may need to call the system call "write" multiple times to reach the size len.
*/
static bool nwrite(int fd, int len, uint8_t *buf) {
  //stores the number written so far in num_written 
  int num_write = write(fd,buf,len);
  int total_written = num_write; 

  while(num_write < len){
    write(fd,buf + total_written,len);
    total_written += num_write;
    printf("\tsucessfully written to the file descriptor...\n");
  }


  return true;
}

/* Through this function call the client attempts to receive a packet from sd 
(i.e., receiving a response from the server.). It happens after the client previously 
forwarded a jbod operation call via a request message to the server.  
It returns true on success and false on failure. 
The values of the parameters (including op, ret, block) will be returned to the caller of this function: 

op - the address to store the jbod "opcode"  
ret - the address to store the info code (lowest bit represents the return value of the server side calling the corresponding jbod_operation function. 2nd lowest bit represent whether data block exists after HEADER_LEN.)
block - holds the received block content if existing (e.g., when the op command is JBOD_READ_BLOCK)

In your implementation, you can read the packet header first (i.e., read HEADER_LEN bytes first), 
and then use the length field in the header to determine whether it is needed to read 
a block of data from the server. You may use the above nread function here.  
*/
static bool recv_packet(int sd, uint32_t *op, uint8_t *ret, uint8_t *block) {
  //memcpy(op,)
  packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
  if (nread(sd,HEADER_LEN,(uint8_t*)packet) == false){
    printf("\tdid not successfully read header...\n ");
    return false; 
  }
  printf("\tvalue is:%d\n",packet->info_code );
  if((packet->info_code & 2) == 2){
    //u_int8_t buf[sizeof(uint32_t) + sizeof(uint8_t) + JBOD_BLOCK_SIZE];
    if (nread(sd, sizeof(uint32_t) + sizeof(uint8_t) + JBOD_BLOCK_SIZE, block) == false)
    {
      printf("\t did not successfully read...\n");
      return false;
    }
  }
  
  //getting first four bytes from buffer 
  if(memcpy(op,&(packet->opcode),sizeof(uint32_t)) == NULL){
    printf("\t did not sucessfully copy op");
    return false; 
  }
  if(memcpy(ret,&(packet->info_code),sizeof(uint8_t)) == NULL){
    printf("\tdid not successfully copy ret...\n");
    return false; 

  }
  // if(memcpy(block,packet->block+ sizeof(uint32_t) + sizeof(u_int8_t) , JBOD_BLOCK_SIZE ) == NULL){
  //   printf("did not successfully copy block");
  //   return false; 
  // }

  printf("successfully received packet ");

  // read(sd, op, sizeof(uint32_t));
  // read(sd,ret,sizeof(uint8_t) );
  // read(sd, block, sizeof(jBOD))
  
  return true;
}



/* The client attempts to send a jbod request packet to sd (i.e., the server socket here); 
returns true on success and false on failure. 

op - the opcode. 
block- when the command is JBOD_WRITE_BLOCK, the block will contain data to write to the server jbod system;
otherwise it is NULL.

The above information (when applicable) has to be wrapped into a jbod request packet (format specified in readme).
You may call the above nwrite function to do the actual sending.  
*/
static bool send_packet(int sd, uint32_t op, uint8_t *block) {

  //packet must be sent in specific format 
  //creation of protocol format
  //entire packet must be 261 bytes
  packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
  packet->opcode = op;
  packet->info_code = sd;

  if (block != NULL)
    memcpy(packet->block, block, JBOD_BLOCK_SIZE); 

  // u_int8_t *buf =  
  //writing to the network
  if(nwrite(sd,261,(uint8_t*)packet) == false){
    printf("\tdid not successfully write to the network\n");
    return false;
  }else{
    printf("\tsuccessfully wrote to the network... \n");
    return true;
  }
  
  

}



/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. 
 * this function will be invoked by tester to connect to the server at given ip and port.
 * you will not call it in mdadm.c
*/
bool jbod_connect(const char *ip, uint16_t port) {
  cli_sd = socket(AF_INET, SOCK_STREAM,0);
    struct sockaddr_in server_info; 
    //configuring
    server_info.sin_family = AF_INET;
    //host to network style
    //converts the unsigned number byte order to network byte order
    server_info.sin_port = htons(port);
    //converts text IP address to binary. 
    if(inet_pton(AF_INET, ip,&server_info.sin_addr) < 1){
        printf("\t error found: could not convert IP Address...\n ");
    }

    //returns connection to server
    if(connect(cli_sd,(struct sockaddr*)&server_info,sizeof(server_info)) == -1  ){
        printf("\t error found: could not connect...\n");
        return false; 


    }else{
        printf("\tconnection success!\n"); 

        return true;

    }

    
}






/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void) {
  //will close socket, sever connection 
  if(close(cli_sd) == -1){
    printf("failed to close ");
    ;
  }
  //resetting socket descriptor to -1....
  else{
    cli_sd = -1; 
    printf("\tsuccessfully disconnected from server...\n");
    ;
  }


}

/* sends the JBOD operation to the server (use the send_packet function) and receives 
(use the recv_packet function) and processes the response. 

The meaning of each parameter is the same as in the original jbod_operation function. 
return: 0 means success, -1 means failure.
*/
int jbod_client_operation(uint32_t op, uint8_t *block) {
  //connection to server established
  //jbod_connect(JBOD_SERVER,JBOD_PORT);
  //how do I pass a socket descriptor to this funciton?
  send_packet(cli_sd,op,block);

  uint8_t *info_code = malloc(sizeof(uint8_t));
  uint32_t *op_code = malloc(sizeof(uint32_t));

  recv_packet(cli_sd,op_code, info_code,block);


  if((*info_code & 3) == 2){
    return 0;
  }

  if((*info_code & 3) == 1){
    return -1; 
  }

  if((*info_code & 3) == 0){
    return 0;
  }
  

  return -1;
}
