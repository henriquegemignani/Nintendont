#include "global.h"
#include "common.h"
#include "string.h"
#include "net.h"
#include "PrimeMemoryDumping.h"
#include "Config.h"

//Defines taken from libogc's network.h and network_wii.c

#define INVALID_SOCKET  (~0)
#define SOCKET_ERROR  (-1)

#define INADDR_ANY 0
#define INADDR_BROADCAST 0xffffffff

#define IPPROTO_IP      0
#define IPPROTO_TCP      6
#define IPPROTO_UDP      17
#define  SOL_SOCKET      0xfff

#define  TCP_NODELAY     0x01
#define TCP_KEEPALIVE  0x02

#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3

#define AF_INET 2

/*
 * Option flags per-socket.
 */
#define  SO_DEBUG 0x0001    /* turn on debugging info recording */
#define  SO_ACCEPTCONN 0x0002 /* socket has had listen() */
#define  SO_REUSEADDR 0x0004 /* allow local address reuse */
#define  SO_KEEPALIVE 0x0008 /* keep connections alive */
#define  SO_DONTROUTE 0x0010 /* just use interface addresses */
#define  SO_BROADCAST 0x0020 /* permit sending of broadcast msgs */
#define  SO_USELOOPBACK 0x0040 /* bypass hardware when possible */
#define  SO_LINGER 0x0080 /* linger on close if data present */
#define  SO_OOBINLINE 0x0100 /* leave received OOB data in line */
#define  SO_REUSEPORT 0x0200 /* allow local address & port reuse */

#define SO_DONTLINGER    (int)(~SO_LINGER)

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF 0x1001 /* send buffer size */
#define SO_RCVBUF 0x1002 /* receive buffer size */
#define SO_SNDLOWAT 0x1003 /* send low-water mark */
#define SO_RCVLOWAT 0x1004 /* receive low-water mark */
#define SO_SNDTIMEO 0x1005 /* send timeout */
#define SO_RCVTIMEO 0x1006 /* receive timeout */
#define SO_ERROR 0x1007 /* get error status and clear */
#define SO_TYPE 0x1008 /* get socket type */

#define MSG_DONTWAIT    0x40

enum {
    IOCTL_SO_ACCEPT = 1,
    IOCTL_SO_BIND,
    IOCTL_SO_CLOSE,
    IOCTL_SO_CONNECT,
    IOCTL_SO_FCNTL,
    IOCTL_SO_GETPEERNAME, // todo
    IOCTL_SO_GETSOCKNAME, // todo
    IOCTL_SO_GETSOCKOPT,  // todo    8
    IOCTL_SO_SETSOCKOPT,
    IOCTL_SO_LISTEN,
    IOCTL_SO_POLL,        // todo    b
    IOCTLV_SO_RECVFROM,
    IOCTLV_SO_SENDTO,
    IOCTL_SO_SHUTDOWN,    // todo    e
    IOCTL_SO_SOCKET,
    IOCTL_SO_GETHOSTID,
    IOCTL_SO_GETHOSTBYNAME,
    IOCTL_SO_GETHOSTBYADDR,// todo
    IOCTLV_SO_GETNAMEINFO, // todo   13
    IOCTL_SO_UNK14,        // todo
    IOCTL_SO_INETATON,     // todo
    IOCTL_SO_INETPTON,     // todo
    IOCTL_SO_INETNTOP,     // todo
    IOCTLV_SO_GETADDRINFO, // todo
    IOCTL_SO_SOCKATMARK,   // todo
    IOCTLV_SO_UNK1A,       // todo
    IOCTLV_SO_UNK1B,       // todo
    IOCTLV_SO_GETINTERFACEOPT, // todo
    IOCTLV_SO_SETINTERFACEOPT, // todo
    IOCTL_SO_SETINTERFACE,     // todo
    IOCTL_SO_STARTUP,           // 0x1f
    IOCTL_SO_ICMPSOCKET = 0x30, // todo
    IOCTLV_SO_ICMPPING,         // todo
    IOCTL_SO_ICMPCANCEL,        // todo
    IOCTL_SO_ICMPCLOSE          // todo
};

struct address {
    unsigned char len;
    unsigned char family;
    unsigned short port;
    unsigned int name;
    unsigned char unused[20];
};

struct bind_params {
    unsigned int socket;
    unsigned int has_name;
    struct address addr;
};

struct sendto_params {
    unsigned int socket;
    unsigned int flags;
    unsigned int has_destaddr;
    struct address addr;
};

struct setsockopt_params {
    u32 socket;
    u32 level;
    u32 optname;
    u32 optlen;
    u8 optval[20];
};

u32 netStart = 0;

void NetInit() {
  netStart = 1;
  while (netStart == 1)
    mdelay(100);
}

u32 NetThread(void *arg) {
  while (!netStart)
    mdelay(100);

  if (!ConfigGetConfig(NIN_CFG_PRIME_DUMP)) {
    netStart = 0;
    return 0;
  }

  char *soDev = "/dev/net/ip/top";
  void *name = heap_alloc_aligned(0, 32, 32);
  memcpy(name, soDev, 32);
  int soFd = IOS_Open(name, 0);
  heap_free(0, name);

  //SOStartup
  IOS_Ioctl(soFd, IOCTL_SO_STARTUP, 0, 0, 0, 0);

  //SOGetHostId
  int ip = 0;
  do {
    mdelay(500);
    ip = IOS_Ioctl(soFd, IOCTL_SO_GETHOSTID, 0, 0, 0, 0);
  } while (ip == 0);

  //Basic network up
  netStart = 0;

  //SOSocket
  unsigned int *params = (unsigned int *) heap_alloc_aligned(0, 12, 32);
  params[0] = AF_INET;
  params[1] = SOCK_STREAM;
  params[2] = IPPROTO_IP;
  int sock = IOS_Ioctl(soFd, IOCTL_SO_SOCKET, params, 12, 0, 0);

  //SOBind
  struct bind_params *bParams = (struct bind_params *) heap_alloc_aligned(0, sizeof(struct bind_params), 32);
  memset(bParams, 0, sizeof(struct bind_params));
  bParams->socket = sock;
  bParams->has_name = 1;
  bParams->addr.len = 8;
  bParams->addr.family = AF_INET;
  bParams->addr.port = 43673;
  bParams->addr.name = INADDR_ANY;
  IOS_Ioctl(soFd, IOCTL_SO_BIND, bParams, sizeof(struct bind_params), 0, 0);

  //SOListen
  params[0] = sock;
  params[1] = 1;
  IOS_Ioctl(soFd, IOCTL_SO_LISTEN, params, 8, 0, 0);
  //Say PPC to continue
  //*(volatile unsigned int*)0x120F0000 = ip;
  //sync_after_write((void*)0x120F0000, 0x20);

  struct sendto_params *sParams = (struct sendto_params *) heap_alloc_aligned(0, sizeof(struct sendto_params), 32);
  PrimeMemoryDump *msg = (PrimeMemoryDump *) heap_alloc_aligned(0, sizeof(struct PrimeMemoryDump), 32);
  ioctlv *sendVec = (ioctlv *) heap_alloc_aligned(0, sizeof(ioctlv) * 2, 32);

  while (1) {
    //SOAccept
    memset(bParams, 0, sizeof(struct bind_params));
    bParams->addr.len = 8;
    bParams->addr.family = AF_INET;
    params[0] = sock;
    int pcSock = IOS_Ioctl(soFd, IOCTL_SO_ACCEPT, params, 4, &bParams->addr, 8);

    //SOSendTo preparation
    memset(sParams, 0, sizeof(struct sendto_params));
    sParams->socket = pcSock;
    sParams->flags = 0;
    sParams->has_destaddr = 0;

    sendVec[0].data = msg;
    sendVec[0].len = sizeof(PrimeMemoryDump);
    sendVec[1].data = sParams;
    sendVec[1].len = sizeof(struct sendto_params);

    while (1) {
      primeMemoryDump(msg);

      if (msg->type != PACKET_TYPE_INVALID) {
        //SOSendTo
        s32 err = IOS_Ioctlv(soFd, IOCTLV_SO_SENDTO, 2, 0, sendVec);

        if (err < 0) {
          break; //Something went wrong
        }
      }
      mdelay(10);
    }

    //Socket disconnected; cleanup
    //SOClose
    STACK_ALIGN(u32, sockPtr, 1, 32);
    *sockPtr = pcSock;
    IOS_Ioctl(soFd, IOCTL_SO_CLOSE, sockPtr, 4, NULL, 0);
  }
  return 0;
}
