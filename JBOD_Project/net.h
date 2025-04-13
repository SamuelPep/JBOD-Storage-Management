#ifndef NET_H_
#define NET_H_

#include <stdint.h>
#include <stdbool.h>
#include "jbod.h"

#define HEADER_LEN (sizeof(uint32_t) + sizeof(uint8_t))
#define JBOD_SERVER "127.0.0.1"
#define JBOD_PORT 3333

int jbod_client_operation(uint32_t op, uint8_t *block);
bool jbod_connect(const char *ip, uint16_t port);
void jbod_disconnect(void);

typedef struct{
  uint32_t opcode; 
  uint8_t  info_code;
  uint8_t block[JBOD_BLOCK_SIZE];
}packet_t;

#endif
