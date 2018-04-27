#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>  //estrutura ifr
#include <netinet/ether.h> //header ethernet
#include <netinet/in.h> //definicao de protocolos
#include <arpa/inet.h> //funcoes para manipulacao de enderecos IP
#include <netinet/in_systm.h> //tipos de dados
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

#define BUFFSIZE 1518

#define DEST_MAC0	0x00
#define DEST_MAC1	0x00
#define DEST_MAC2	0x00
#define DEST_MAC3	0x00
#define DEST_MAC4	0x00
#define DEST_MAC5	0x00

#define SRC_MAC0	0xb4
#define SRC_MAC1	0xb6
#define SRC_MAC2	0x76
#define SRC_MAC3	0x43
#define SRC_MAC4	0x8d
#define SRC_MAC5	0x2a
	
unsigned char buff1[BUFFSIZE]; // buffer de recepcao

int sockd;
int on;
struct ifreq ifr;

int main(int argc,char *argv[])
{
	struct ether_header *eh = (struct ether_header *) buff1;
	struct iphdr *iph = (struct iphdr *) (buff1 + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buff1 + sizeof(struct iphdr) + sizeof(struct ether_header));
	FILE *fp;
	short ipFlags = 0;
	short ipOffset;
	char moreFragments;
    if((sockd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
       printf("Erro na criacao do socket.\n");
       exit(1);
    }

	strcpy(ifr.ifr_name, "wlp2s0");
	if(ioctl(sockd, SIOCGIFINDEX, &ifr) < 0)
		printf("erro no ioctl!");
	ioctl(sockd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sockd, SIOCSIFFLAGS, &ifr);

	int arrOffset[44];
	int counter = 0;
	int lastFragment = -1;
	char fullData[65365];
	// recepcao de pacotes
	while (1) {
   		recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
   		if(eh->ether_dhost[0] == DEST_MAC0 &&
   			eh->ether_dhost[1] == DEST_MAC1 &&
   			eh->ether_dhost[2] == DEST_MAC2 &&
   			eh->ether_dhost[3] == DEST_MAC3 &&
   			eh->ether_dhost[4] == DEST_MAC4 &&
   			eh->ether_dhost[5] == DEST_MAC5 &&
   			eh->ether_shost[0] == SRC_MAC0 &&
   			eh->ether_shost[1] == SRC_MAC1 &&
   			eh->ether_shost[2] == SRC_MAC2 &&
   			eh->ether_shost[3] == SRC_MAC3 &&
   			eh->ether_shost[4] == SRC_MAC4 &&
   			eh->ether_shost[5] == SRC_MAC5)
   		{
	   		printf("pacote novo\n");
			printf("ethernet frame\n");
			printf("MAC Destino: %x:%x:%x:%x:%x:%x \n", eh->ether_dhost[0],eh->ether_dhost[1],eh->ether_dhost[2],eh->ether_dhost[3],eh->ether_dhost[4],eh->ether_dhost[5]);
			printf("MAC Origem:  %x:%x:%x:%x:%x:%x \n", eh->ether_shost[0],eh->ether_shost[1],eh->ether_shost[2],eh->ether_shost[3],eh->ether_shost[4],eh->ether_shost[5]);
			printf("ether type: %x\n",eh->ether_type);
   			printf("ip frame\n");
   			printf("id: %d\n", iph->id);
   			ipFlags = ntohs(iph->frag_off);
   			ipOffset = ipFlags << 3;
   			ipOffset = (ipOffset >> 3) * 8;
   			moreFragments = (ipFlags >> 13) & 1;

   			if(!moreFragments && ntohs(iph->tot_len) < 1500){
   				printf("ultimo pacote\n");
   				if(counter == 0)
   					arrOffset[counter] = 0;
   				else
   					arrOffset[counter] = (ipOffset / 1480);
   			}else{
   				printf("pacotes intermediarios\n");
   				arrOffset[counter] = ipOffset / 1480;
   			}

   			if(!moreFragments)
   				lastFragment = arrOffset[counter]; 

   			printf("array Offset: %d\n", arrOffset[counter]);
   			printf("last Fragment: %d\n", lastFragment);
   			
   			printf("ip Offset: %d \n", ipOffset);
   			printf("ip protocol: %d \n", iph->protocol);
   			printf("total length: %d\n", ntohs(iph->tot_len));
   			//verificar essa linha para cabeçalho
   			//int udpHeaderSize = ipOffset ? 0 : 8;
   			int udpHeaderSize = 0;
   			int etherHeaderSize = 14;
   			int ipHeaderSize = 20;
   			int dataLength = ntohs(iph->tot_len) - udpHeaderSize - ipHeaderSize;
   			printf("dataLength: %d \n", dataLength); 
   			char *data = (char *) (buff1 + ipHeaderSize + etherHeaderSize + udpHeaderSize);
   			printf("data\n");

   			int nFragment = arrOffset[counter];
   			int startData = arrOffset[counter] * 1480;
   			int endData = 0;
   			if(!moreFragments)
   				endData = startData + dataLength;
   			else
   				endData = startData + 1480;

   			int j = 0;
   			printf("startData: %d\n", startData);
   			printf("endData: %d\n", endData);
   			for(int i=startData;i<endData;i++){
   				//printf("laco for position: %d\n", i);
   				fullData[i] = data[j];
   				//printf("%c", fullData[(counter*1480)+i]);
   				j++;
   			}




   			printf("more fragments: %d\n", moreFragments);
   			printf("contador de pacotes: %d\n", counter);
   			counter++;
   			if(!moreFragments && (counter == lastFragment+1)){
   				printf("comeco a colocar dados no arquivo\n");
   				int sizeFullData = (int)strlen(fullData+8);
   				printf("size of fullData %d \n", sizeFullData);
	   			fp = fopen("recebido.txt", "w");

	   			for (int i = 0; i < sizeFullData; i++)
	   			{
	   				//printf("posição do arquivo: %d -> ", i);
	   				fputc(fullData[i+8], fp);
	   				//printf("%c \n", fullData[i+8]);
	   			}
	   			printf("\n");
	   			fclose(fp);
	   			printf("fim data\n");
	   			arrOffset[44];
	   			counter = 0;
	   			lastFragment = -1;
   			}

   			printf("\n\n");
   		}
	}
}