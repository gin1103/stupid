#include <stdio.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include "skbuff.h"
#include "udp.h"
#include "utils.h"
#include "ip.h"

void udp_rcv(struct sk_buff *skb)
{
	struct udphdr *uh;

	uh = skb->h.uh = (struct udphdr *) skb->data;
	skb->data += 8;

	/* checksum is related to a pseudo L3 header, not checked for now */

	printf("udp sport: %5d, dport %5d, length %5d\n",
	       ntohs(uh->source), ntohs(uh->dest), ntohs(uh->len));

	data_dump("udp: data received", skb->data, ntohs(uh->len));

	/* add to sock's received queue */
	/* or we can run a test application program here */

}

void udp_send(unsigned char *data, unsigned int len, struct sock *sock)
{
	struct udphdr *uh;
	struct sk_buff *skb;
	struct pseudo_header *ph;
	int i;

	skb = alloc_skb(sock->nic);
	skb->protocol = IPPROTO_UDP;
	skb->len = len + 8;
	skb->sock = sock;
	/* the space between head and data should be enough for L2, L3 and L4
	   headers */
	/* and we assume that fragmentation is unnecessary */
	if (len > 1000)
	{
		printf("sorry, the data you want to transmit is too long\n");
		goto out;
	}

	/* udp payload */
	skb->data = skb->head + 42;
	for (i = 0; i < len; i++)
		skb->data[i] = data[i];
	/* udp header */
	skb->data -= 8;
	uh = (struct udphdr *) skb->data;
	uh->source = htons(48250);
	uh->dest = sock->dest.port;	/* send the datagram to echo server */
	uh->len = htons(skb->len);
	uh->check = 0;

	ph = (struct pseudo_header *) (skb->data - 12);
	ph->saddr = sock->nic->ip;
	ph->daddr = sock->dest.ip;
	ph->resv = 0;
	ph->protocol = skb->protocol;
	ph->len = uh->len;
	uh->check = in_checksum((__u16 *)ph, 12 + skb->len);

	ip_send(skb);

out:
	return;
}
