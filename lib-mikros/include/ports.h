#ifndef PORTS_H_
#define PORTS_H_

#include "stdint.h"

#define PORTM_BYTE 0
#define PORTM_WORD 1
#define PORTM_LONG 2

uint32_t require_port(uint16_t port);

uint32_t port_in(uint32_t mode, uint16_t port);
uint32_t port_out(uint32_t mode, uint16_t port, uint32_t value);

#endif /* PORTS_H_ */
