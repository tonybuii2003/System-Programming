/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * The "Xacto" transactional store protocol.
 *
 * This header file specifies the format of communication between the
 * Xacto server and its clients.  We will use the term "packet" to refer
 * to a single message sent at the protocol level.  A full-duplex,
 * stream-based (i.e. TCP) connection is used between a client and the
 * server.  Communication is effected by the client and server sending
 * "packets" to each other over this connection.  Each packet consists
 * of a fixed-length header, with fields in network byte order,
 * followed by a payload whose length is specified in the header.
 *
 * The following are the packet types in the protocol:
 *
 * Client-to-server requests:
 *   PUT:     Put a key/value mapping in the store
 *            (sends request serial #, key, and value)
 *	      (reply echoes serial # and returns status)
 *   GET:     Get the store value corresponding to a key
 *            (sends request serial # and key)
 *	      (reply echoes serial # and returns status and value)
 *   COMMIT:  Try to commit a transaction
 *            (sends request serial #)
 *            (reply echoes serial # and returns status)
 *
 * Server-to-client responses:
 *   REPLY:
 *            (echoes request serial #, returns status and possibly also
 *             value, if the original request was a GET)
 *
 * Data packets
 *	      (contain a key or value; must follow an associated
 *             request or response packet and contain the serial number
 *             for that packet):
 *
 *   KEY:     Contains the key for a PUT or GET request.
 *   VALUE:   Contains the value for a PUT request or GET reply.
 *
 * Data objects are sent immediately following the request or reply packet
 * with which they are associated.  A data object is transmitted by first
 * sending a fixed-size header, which specifies the length of the data payload,
 * followed by the data payload itself, which consists of exactly the number
 * of bytes specified in the payload_length field of the header.
 */

/*
 * Packet types.
 */
typedef enum
{
    XACTO_NO_PKT, // Not used
    XACTO_PUT_PKT,
    XACTO_GET_PKT,
    XACTO_KEY_PKT,
    XACTO_VALUE_PKT,
    XACTO_COMMIT_PKT,
    XACTO_REPLY_PKT
} XACTO_PACKET_TYPE;

/*
 * Structure of a packet.
 */
typedef struct
{
    uint8_t type;            // Type of the packet
    uint8_t status;          // Status (in reply packet)
    uint8_t null;            // Whether payload is NULL (in data packet)
    uint32_t serial;         // Serial # of the associated request
    uint32_t size;           // Payload size (for data packet)
    uint32_t timestamp_sec;  // Seconds field of time packet was sent
    uint32_t timestamp_nsec; // Nanoseconds field of time packet was sent
} XACTO_PACKET;

/*
 * Send a packet header, followed by an associated data payload, if any.
 * Multi-byte fields in the packet header are stored in network byte oder.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size part of the packet, with multi-byte fields
 *   in network byte order
 * @param data  The payload for data packet, or NULL.  A NULL value used
 *   here for a data packet specifies the transmission of a special null
 *   data value, which has no content.
 * @return  0 in case of successful transmission, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 */
int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data);

/*
 * Receive a packet, blocking until one is available.
 * The returned structure has its multi-byte fields in network byte order.
 *
 * @param fd  The file descriptor from which the packet is to be received.
 * @param pkt  Pointer to caller-supplied storage for the fixed-size
 *   portion of the packet.
 * @param datap  Pointer to variable into which to store a pointer to any
 *   payload received.
 * @return  0 in case of successful reception, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 *
 * If the returned payload pointer is non-NULL, then the caller assumes
 * responsibility for freeing the storage.
 */
int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap);

#endif
