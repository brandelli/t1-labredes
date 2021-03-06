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

//struct utilizada para o checksum do udp
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
	char *pseudo_packet;
	char *tempData = NULL;
	int udpMode;
	int fileOffset = 0;
	int fragOffset = 0;

	//recebe o segundo argumento
	udpMode = atoi(argv[2]);
	strcpy(ifTeste, argv[0]);
	strcpy(ifName, argv[1]);
	FILE *fp;

	char *buffer = NULL;
	size_t size = 0;

	fp = fopen("dont.txt", "r");
	//fp = fopen("fragment.txt", "r");
    
    //vai ao final do arquivo
	fseek(fp, 0, SEEK_END);
	//verifica o tamanho do arquivo
	size = ftell(fp);

	//volta o ponteiro do arquivo ao inicio
	rewind(fp);

	//aloca o tamanho do arquivo + 1, para terminador de string
	buffer = malloc((size + 1) * sizeof(*buffer));

	//le o arquivo
	fread(buffer, size, 1, fp);

	//insere o final de input
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
		int counter = 0;
		//faz o preenchimento da estrutura temporaria para o payload
		if(fileOffset){
			printf("resto fragmentos\n");
			tempData = (char *)calloc(1481, sizeof(char));
			while(counter<1480 && (int) strlen(tempData) < 1480){
				tempData[counter] = (char)buffer[fileOffset + counter];
				counter++;
			}
			tempData[1480] = '\0';
		}else{
			printf("primeiro fragmento\n");
			tempData = (char *)calloc(1472, sizeof(char));
			int counter = 0;
			while(counter<1472){
				tempData[counter] = buffer[counter];
				counter++;
			}
		}
		printf("tamanho tempData %d \n", (int) strlen(tempData));
		printf("fileOffset %d \n", fileOffset);

		//inicia a montagem dos cabeçalhos
		tx_len = 0;
		memset(sendbuf, 0, BUF_SIZ);
		//ethernet
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
		eh->ether_type = htons(ETH_P_IP);

		//quantos dados foram inseridos no pacote
		tx_len += sizeof(struct ether_header);
		//verificacoes de mais fragmentos e offset do arquivo
		int moreFragments = (fileOffset + 1480) > size ? 0 : 1;
		int fOffset = fileOffset ? fileOffset + 8 : fileOffset;
		printf("%i\n", fileOffset); 
		uint16_t frag = (0 << 15) + (0 << 14) + (moreFragments << 13) +  fragOffset/8;
		
		//ip
		iph->ihl = 5;
		iph->version = 4;
		iph->tos = 16;
		iph->id = htons(171);
		iph->ttl = 255;
		iph->protocol = udpMode ? 17 : 6; // UDP
		iph->frag_off = htons (frag);
		iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
		// iph->saddr = inet_addr("192.168.0.112");
		iph->daddr = iph->saddr;
		iph->check = 0;
		
		//tamanho do pacote
		if(fileOffset)
			iph->tot_len = htons(sizeof(struct iphdr) + (int) strlen(tempData));
		else
			iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + (int) strlen(tempData));

		printf("ether :%i\n", (int) sizeof(struct ether_header));
		printf("iphdr :%i\n", (int) sizeof(struct iphdr));
		printf("udp :%i\n", (int) sizeof(struct udphdr));
		printf("data :%i\n", (int) strlen(tempData));
		printf("ip+udp+data: %i\n", (int)(sizeof(struct iphdr) + sizeof(struct udphdr) + (int)strlen(tempData)));
		
		//calculo do checksum do ip
		iph->check = in_cksum((unsigned short *)iph, sizeof(struct iphdr));
		tx_len += sizeof(struct iphdr);

		if(udpMode){
			//caso seja udp
			char *payload;
			//não é primeiro fragmento
			if(fileOffset){
				payload = (char *) (sendbuf + sizeof(struct ether_header) + sizeof(struct iphdr));
				strcpy(payload,tempData);
			}else{
				//primeiro fragento
				struct udphdr *udph = (struct udphdr *) (sendbuf + sizeof(struct iphdr) + sizeof(struct ether_header));
				payload = (char *) (sendbuf + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));
				udph->source = htons(3423);
				udph->dest = htons(5342);
				udph->check = 0;
				tx_len += sizeof(struct udphdr);
				udph->len = htons(sizeof(struct udphdr) + (int) strlen(buffer));
				//montagem do pseudo cabeçalho upd, para checksum
				psHeader.srcAddr = iph->saddr;
				psHeader.dstAddr = iph->daddr;
				psHeader.zero = 0;
				psHeader.protocol = 17;
				psHeader.len = udph->len;
				pseudo_packet = (char *) malloc((int) (sizeof(struct pseudoHeader) + sizeof(struct udphdr) + (int)strlen(buffer)));
				memset(pseudo_packet, 0, sizeof(struct pseudoHeader) + sizeof(struct udphdr) + strlen(buffer));
				memcpy(pseudo_packet, (char *) &psHeader, sizeof(struct pseudoHeader));
				memcpy(pseudo_packet + sizeof(struct pseudoHeader), udph, sizeof(struct udphdr) + strlen(buffer));
				memcpy(pseudo_packet + sizeof(struct pseudoHeader) + sizeof(struct udphdr), buffer,strlen(buffer));
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
		socket_address.sll_ifindex = if_idx.ifr_ifindex;
		socket_address.sll_halen = ETH_ALEN;
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
		printf("vou enviar pacote\n\n");
		if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0){
		    printf("Send failed\n");
		    return 0;
		}

	
	}
	
	return 0;
}
