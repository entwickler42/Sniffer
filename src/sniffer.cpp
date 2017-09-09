/***************************************************************************
 *   Copyright (C) 2004 by l4t3n8                                          *
 *   mailbockx@freenet.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static bool run;
static int cs;

using namespace std;

//-----------------------------------------------------------------------------
void sigfunc(int sig)
{
	if(sig = SIGINT) 
	{
		cout << "GOT SIGINT, TERMINATING SOON ! \n";	
		run = false;
	}
}

//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{	
	unsigned int b_size;
	struct ifreq if_req;
	struct iphdr  *ip;
	struct tcphdr *tcp;
	struct ethhdr *eth;

	run = true;
	
	memset((char*)&if_req,0,sizeof(struct ifreq));
			
	signal(SIGINT,sigfunc);
		
	//cs = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	cs = socket(AF_INET, SOCK_RAW, IPPROTO_TCP); // WORKS FOR SNIFFING
	
	if(cs < 0) 
	{
		cout << "CAN NOT GET SOCKET - NOTICE: YOU MUST HAVE ROOT PRIVILEGS" << endl;
		exit(1);
	}

	if(argc < 2 )	
		strcpy(if_req.ifr_ifrn.ifrn_name,"eth0");
	else 
		strcpy(if_req.ifr_ifrn.ifrn_name,argv[1]);
		
	cout << "USING INTERFACE : " << if_req.ifr_ifrn.ifrn_name << endl;
	
	if( ioctl(cs,SIOCGIFMTU,&if_req) == -1 ) 
	{		
		cout << "MTU : 1500 ( default used for buffer size )" << endl;
		b_size = 1500;
	}
	else
	{	
		cout << "MTU : " << if_req.ifr_ifru.ifru_mtu << " ( used for buffer size )" <<endl;
		b_size = if_req.ifr_ifru.ifru_mtu;
	}
		
	char* buf = new char[b_size];		
	
	eth = (struct ethhdr*) (buf);
	ip 	= (struct iphdr*)  (buf+sizeof(struct ethhdr));
	tcp = (struct tcphdr*) (buf+sizeof(struct iphdr)+sizeof(struct ethhdr));
	
	unsigned int str_len = sizeof(struct tcphdr)+sizeof(struct iphdr)+sizeof(struct ethhdr);
	

    while(run)
	{				
		if(read(cs,buf,b_size) > 0) 
		{		
			cout << inet_ntoa(*(struct in_addr*)&ip->saddr) << "\t--->\t" << inet_ntoa(*(struct in_addr*)&ip->daddr) << endl;
			//write(0,buf+str_len,ntohs(ip->tot_len)-str_len);
		}
		memset(buf,0,b_size);
	}
	
	delete buf; close(cs);	
	cout << "\nTERMINATED\n";	
	return EXIT_SUCCESS;
}
