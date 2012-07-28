#ifndef _NETWORK_H_
#define _NETWORK_H_

// TODO: winsock support
#ifdef WIN32
#warning "Winsock not supported yet!"
#else
#include <sys/time.h>
#include <sys/types.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void net_report_layer(s32 x, s32 y, u8 position);
void net_report_unlayer(s32 x, s32 y, u8 position);

#endif /* _NETWORK_H_ */

