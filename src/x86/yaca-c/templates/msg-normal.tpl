/*
 * Temporary, auto-generated file. Do not edit.
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define YE(a) ((unsigned char *) (a))

/*
 * HDM: Header-Define Message. Defines a message handler which is executed on message reception
 * HDR: Header-Define RTR. Defines a remote transmission request handler
 * HDS: Header-Define Status. Defines a status message which can be emitted
 *
 */

#define HDM(a) __msg_##a
#define HDR(a) __rtr_##a
#define HDS(a) __attribute__((weak)) __msg_##a {}
#define DM(a) <NODENAME>::__msg_##a
#define DR(a) <NODENAME>::__rtr_##a
#define yc_send(a, b) R##a::__msg_##b
#define yc_rtr(a, b) R##a::__rtr_##b
#define yc_status(a) <NODENAME>::__rtr_##a()

#endif /* _MESSAGES_H_ */

