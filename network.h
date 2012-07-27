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

#endif /* _NETWORK_H_ */

