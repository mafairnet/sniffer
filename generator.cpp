/*Writing some small packet generator
 */

#include "generator.h"
#include "sniff.h"


void 
*gensiprtp(void *params) {
	Generator *gen = new FILE_LINE(9001) Generator("1.1.1.1", "2.2.2.2");
	//send test data 
	struct udphdr2 udph;
	udph.dest = 5060;
	udph.len = 8;
	udph.source = 5060;
	udph.check = 0;

	//char buf[4092];

	gen->send((char*)&udph, 8);
	
	return NULL;
}

void 
Generator::socket_broadcast(int sd)
{
	const int one = 1;
 
	if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST,(char *)&one, sizeof(one)) == -1)
	{
		syslog(LOG_ERR, "[socket_broadcast] can't set SO_BROADCAST option\n");
		/* non fatal error */
	}
}
 
void 
Generator::socket_iphdrincl(int sd)
{
	const int one = 1;
 
	if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL,(char *)&one, sizeof(one)) == -1)
	{
		syslog(LOG_ERR, "[socket_broadcast] can't set IP_HDRINCL option\n");
		/* non fatal error */
	}
}
 
Generator::Generator(const char *src, const char *dst) {
 
	sockraw = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	 
	socket_broadcast(sockraw);
	/* set SO_IPHDRINCL option */
	socket_iphdrincl(sockraw);

 
	src_addr.sin_family = AF_INET;
	src_addr.sin_addr.s_addr = inet_addr(src);
	src_addr.sin_port = htons(9095);
	memset(&src_addr.sin_zero, '\0', 8);
 
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(dst);
	dest_addr.sin_port = htons(9095);
	memset(&dest_addr.sin_zero, '\0', 8);
   /* 
	if(bind(sockraw, (struct sockaddr *)&src_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		//return -1;
	}
*/
	//return 0;
}

int
Generator::send(char *data, int datalen) {

	struct iphdr2 *iphdr;

	iphdr = (struct iphdr2 *)generator_packet;
	memset(iphdr, 0, sizeof(iphdr2));
	iphdr->version = 4;
	iphdr->tos = 0;
	iphdr->id = htons(100);
	iphdr->frag_off = 0;
	iphdr->ttl = 120;
	iphdr->protocol = 17;
	iphdr->check = 0;
	iphdr->tot_len = htons(sizeof(struct iphdr2) + datalen);
	iphdr->ihl = 5;
	iphdr->check = 0;
	iphdr->saddr = src_addr.sin_addr.s_addr;
	iphdr->daddr = dest_addr.sin_addr.s_addr;
	memcpy(generator_packet + sizeof(struct iphdr2), data, datalen);

	int res;
	if((res = sendto(sockraw, generator_packet, datalen + sizeof(struct iphdr2), 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))) == -1)
	{
		printf("msglen[%lu]\n", datalen + sizeof(struct iphdr2));
		perror("sendto");
	}
	return res;
}
