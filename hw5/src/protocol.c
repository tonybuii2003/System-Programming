#include "protocol.h"
#include <stdlib.h>
#include "csapp.h"
#include "debug.h"
int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data)
{
    uint32_t converted_size = htonl(pkt->size);
    ssize_t bytes_written;
    bytes_written = Rio_writen(fd, pkt, sizeof(XACTO_PACKET));
    if (bytes_written != sizeof(XACTO_PACKET))
    {
        return -1;
    }
    if (data != NULL && converted_size > 0)
    {
        bytes_written = Rio_writen(fd, data, converted_size);
        if (bytes_written != converted_size)
        {
            return -1;
        }
    }
    return 0;
}
int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap)
{
    ssize_t bytes_read;
    bytes_read = Rio_readn(fd, pkt, sizeof(XACTO_PACKET));
    if (bytes_read != sizeof(XACTO_PACKET))
    {
        return -1;
    }
    uint32_t converted_size = ntohl(pkt->size);
    if (ntohl(pkt->size))
    {
        *datap = Malloc(bytes_read);
        bytes_read = Rio_readn(fd, *datap, converted_size);
        if (bytes_read != converted_size)
        {
            Free(*datap);
            return -1;
        }
    }
    return 0;
}
