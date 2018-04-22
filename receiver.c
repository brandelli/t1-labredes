/*-------------------------------------------------------------*/
/* Exemplo Socket Raw - Captura pacotes recebidos na interface */
/*-------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

/* Diretorios: net, netinet, linux contem os includes que descrevem */
/* as estruturas de dados do header dos protocolos   	  	        */

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

// Atencao!! Confira no /usr/include do seu sisop o nome correto
// das estruturas de dados dos protocolos.
	
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
    /* Criacao do socket. Todos os pacotes devem ser construidos a partir do protocolo Ethernet. */
    /* De um "man" para ver os parametros.*/
    /* htons: converte um short (2-byte) integer para standard network byte order. */
    if((sockd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
       printf("Erro na criacao do socket.\n");
       exit(1);
    }

	// O procedimento abaixo eh utilizado para "setar" a interface em modo promiscuo
	strcpy(ifr.ifr_name, "wlp2s0");
	if(ioctl(sockd, SIOCGIFINDEX, &ifr) < 0)
		printf("erro no ioctl!");
	ioctl(sockd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sockd, SIOCSIFFLAGS, &ifr);

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
			// impress�o do coneudo - exemplo Endereco Destino e Endereco Origem
			printf("ethernet frame\n");
			printf("MAC Destino: %x:%x:%x:%x:%x:%x \n", eh->ether_dhost[0],eh->ether_dhost[1],eh->ether_dhost[2],eh->ether_dhost[3],eh->ether_dhost[4],eh->ether_dhost[5]);
			printf("MAC Origem:  %x:%x:%x:%x:%x:%x \n", eh->ether_shost[0],eh->ether_shost[1],eh->ether_shost[2],eh->ether_shost[3],eh->ether_shost[4],eh->ether_shost[5]);
			printf("ether type: %x\n",eh->ether_type);
   			printf("ip frame\n\n");
   			printf("id: %d\n", iph->id);
   			printf("ip protocol: %d \n", iph->protocol);
   			printf("total length: %d\n\n", ntohs(iph->tot_len));
   			int dataLength = ntohs(iph->tot_len) - 8 - 20;
   			printf("dataLength: %d \n", dataLength); 
   			char *data = (char *) (buff1 + sizeof(struct iphdr) + sizeof(struct ether_header) + sizeof(struct udphdr));
   			printf("data\n");
   			fp = fopen("recebido.txt", "w");
   			for (int i = 0; i < dataLength; ++i)
   			{
   				fputc(data[i], fp);
   				printf("%c", data[i]);
   			}
   			fclose(fp);
   			printf("fim data\n");
   		}
	}
}