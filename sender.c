#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>

#define MY_DEST_MAC0	0x00
#define MY_DEST_MAC1	0x00
#define MY_DEST_MAC2	0x00
#define MY_DEST_MAC3	0x00
#define MY_DEST_MAC4	0x00
#define MY_DEST_MAC5	0x00

#define BUF_SIZ		1514

unsigned short in_cksum(unsigned short *addr,int len)
{
        register int sum = 0;
        u_short answer = 0;
        register u_short *w = addr;
        register int nleft = len;

        /*
         * Our algorithm is simple, using a 32 bit accumulator (sum), we add
         * sequential 16 bit words to it, and at the end, fold back all the
         * carry bits from the top 16 bits into the lower 16 bits.
         */
        while (nleft > 1)  {
                sum += *w++;
                nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nleft == 1) {
                *(u_char *)(&answer) = *(u_char *)w ;
                sum += answer;
        }

        /* add back carry outs from top 16 bits to low 16 bits */
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);                     /* add carry */
        answer = ~sum;                          /* truncate to 16 bits */
        return(answer);
}

struct pseudoHeader{
  uint32_t srcAddr;
  uint32_t dstAddr;
  uint8_t zero;
  uint8_t protocol;
  uint16_t len;
};

int main(int argc, char *argv[])
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	struct pseudoHeader psHeader;
	char ifName[IFNAMSIZ];
	char ifTeste[IFNAMSIZ];
	char *aux;
	char *pseudo_packet;
	#pragma pack(1)
	char *tempData = NULL;
	int udpMode;
	int fileOffset = 0;
	int fragOffset = 0;
	//a4:1f:72:f5:90:c2 canto 
	//a4:1f:72:f5:90:52	menos canto
	udpMode = atoi(argv[2]);
	strcpy(ifTeste, argv[0]);
	strcpy(ifName, argv[1]);
	FILE *fp;

	char *buffer = NULL;
	size_t size = 0;

	//fp = fopen("dont.txt", "r");
	fp = fopen("fragment.txt", "r");
	/* Get the buffer size */
	fseek(fp, 0, SEEK_END); /* Go to end of file */
	size = ftell(fp); /* How many bytes did we pass ? */

	/* Set position of stream to the beginning */
	rewind(fp);

	buffer = malloc((size + 1) * sizeof(*buffer)); /* size + 1 byte for the \0 */

	/* Read the file into the buffer */
	fread(buffer, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */

	/* NULL-terminate the buffer */
	buffer[size] = '\0';
	printf("tamanho arquivo: %i bytes\n", (int)size);

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	struct ifreq if_ip;
	memset(&if_ip, 0, sizeof(struct ifreq));
	strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFADDR, &if_ip) < 0)
	    perror("SIOCGIFADDR");


	while(fileOffset < size){
		char *tempData;	
		if(fileOffset){
			printf("resto fragmentos\n");
			tempData = (char *)calloc(1481, sizeof(char));
			int teste = 0;
			while(teste<1480 && (int) strlen(tempData) < 1480){
				tempData[teste] = (char)buffer[fileOffset + teste];
				teste++;
			}
			tempData[1480] = '\0';
			printf("str tempdata %d \n", (int) strlen(tempData));
		}else{
			printf("primeiro fragmento\n");
			tempData = (char *)calloc(1472, sizeof(char));
			int teste = 0;
			while(teste<1472){
				tempData[teste] = buffer[teste];
				teste++;
			}
		}
		printf("tamanho tempData %d \n", (int) strlen(tempData));
		printf("fileOffset %d \n", fileOffset);
		//printf("arquivo  %s\n", tempData);
		tx_len = 0;
		/* Construct the Ethernet header */
		memset(sendbuf, 0, BUF_SIZ);
		/* Ethernet header */
		eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
		eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
		eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
		eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
		eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
		eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
		eh->ether_dhost[0] = MY_DEST_MAC0;
		eh->ether_dhost[1] = MY_DEST_MAC0;
		eh->ether_dhost[2] = MY_DEST_MAC0;
		eh->ether_dhost[3] = MY_DEST_MAC0;
		eh->ether_dhost[4] = MY_DEST_MAC0;
		eh->ether_dhost[5] = MY_DEST_MAC0;
		/* Ethertype field */
		eh->ether_type = htons(ETH_P_IP);
		tx_len += sizeof(struct ether_header);
		int moreFragments = (fileOffset + 1480) > size ? 0 : 1;
		int fOffset = fileOffset ? fileOffset + 8 : fileOffset;
		printf("%i\n", fileOffset); 
		uint16_t frag = (0 << 15) + (0 << 14) + (moreFragments << 13) +  fragOffset/8;
		/* IP Header */
		iph->ihl = 5;
		iph->version = 4;
		iph->tos = 16; // Low delay
		iph->id = htons(171);
		iph->ttl = 255; // hops
		iph->protocol = udpMode ? 17 : 6; // UDP
		iph->frag_off = htons (frag);
		iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
		// iph->saddr = inet_addr("192.168.0.112");
		iph->daddr = iph->saddr;
		iph->check = 0;
		/* Length of IP payload and header */
		if(fileOffset)
			iph->tot_len = htons(sizeof(struct iphdr) + (int) strlen(tempData));
		else
			iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + (int) strlen(tempData));

		printf("ether :%i\n", (int) sizeof(struct ether_header));
		printf("iphdr :%i\n", (int) sizeof(struct iphdr));
		printf("udp :%i\n", (int) sizeof(struct udphdr));
		printf("data :%i\n", (int) strlen(tempData));
		printf("ip+udp+data: %i\n", (int)(sizeof(struct iphdr) + sizeof(struct udphdr) + (int)strlen(tempData)));
		/* Calculate IP checksum on completed header */
		iph->check = in_cksum((unsigned short *)iph, sizeof(struct iphdr));
		tx_len += sizeof(struct iphdr);

		if(udpMode){
			char *payload;
			if(fileOffset){
				payload = (char *) (sendbuf + sizeof(struct ether_header) + sizeof(struct iphdr));
				strcpy(payload,tempData);
			}else{
				struct udphdr *udph = (struct udphdr *) (sendbuf + sizeof(struct iphdr) + sizeof(struct ether_header));
				payload = (char *) (sendbuf + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));
				udph->source = htons(3423);
				udph->dest = htons(5342);
				udph->check = 0; // skip
				tx_len += sizeof(struct udphdr);
				udph->len = htons(sizeof(struct udphdr) + (int) strlen(buffer));
				psHeader.srcAddr = iph->saddr; //32 bit format of source address
				psHeader.dstAddr = iph->daddr; //32 bit format of source address
				psHeader.zero = 0; //8 bit always zero
				psHeader.protocol = 17; //8 bit TCP protocol
				psHeader.len = udph->len;
				pseudo_packet = (char *) malloc((int) (sizeof(struct pseudoHeader) + sizeof(struct udphdr) + (int)strlen(buffer)));
				memset(pseudo_packet, 0, sizeof(struct pseudoHeader) + sizeof(struct udphdr) + strlen(buffer));
				memcpy(pseudo_packet, (char *) &psHeader, sizeof(struct pseudoHeader));
				//Copy tcp header + data to fake TCP header for checksum
				memcpy(pseudo_packet + sizeof(struct pseudoHeader), udph, sizeof(struct udphdr) + strlen(buffer));
				memcpy(pseudo_packet + sizeof(struct pseudoHeader) + sizeof(struct udphdr), buffer,strlen(buffer));
				//Set the TCP header's check field
				udph->check = (in_cksum((unsigned short *) pseudo_packet, (int) (sizeof(struct pseudoHeader) + 
				          sizeof(struct udphdr) +  (int)strlen(buffer))));
				strcpy(payload,tempData);
			}
			printf("payload%i\n", (int)strlen(tempData));
			tx_len += strlen(tempData);
			printf("tx_len: %d \n", tx_len);
		}else{
			//tcp
		}
		/* Index of the network device */
		socket_address.sll_ifindex = if_idx.ifr_ifindex;
		/* Address length*/
		socket_address.sll_halen = ETH_ALEN;
		/* Destination MAC */
		socket_address.sll_addr[0] = MY_DEST_MAC0;
		socket_address.sll_addr[1] = MY_DEST_MAC1;
		socket_address.sll_addr[2] = MY_DEST_MAC2;
		socket_address.sll_addr[3] = MY_DEST_MAC3;
		socket_address.sll_addr[4] = MY_DEST_MAC4;
		socket_address.sll_addr[5] = MY_DEST_MAC5;
		if(fileOffset)
			fileOffset+=1480;
		else
			fileOffset+=1472;

		fragOffset +=1480;
		/* Send packet */
		printf("vou enviar pacote\n\n");
		if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0){
		    printf("Send failed\n");
		    return 0;
		}

	
	}
	
	return 0;
}
