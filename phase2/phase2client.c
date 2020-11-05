// Author : Dr. Foster
// Purpose : demonstration of winsock API using a simple server/client
//
//
//
//  Citation : Based off of sample code found at https://www.binarytides.com/winsock-socket-programming-tutorial/

#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

// This pragma ONLY works in Visual Studio. You will need to manually link this library when using something else.
#pragma comment(lib,"ws2_32.lib") //Winsock Library - don't touch this.

CHAR ipaddr[INET_ADDRSTRLEN] = "127.0.0.1"; // CE-480 This is the hard-coded address for the *server*, NOT this client. You will need to change this to talk to another computer
//NOTE - If your project is using Unicode instead of ASCII, you will probably need to change CHAR to WCHAR


// create some constants for error codes if the program dies... helps figure out where things went wrong.
#define ERROR_WINSOCK_INIT_FAILURE				1;
#define ERROR_WINSOCK_SOCKET_CREATE_FAILURE		2;
#define ERROR_WINSOCK_SOCKET_CONNECT_FAILURE	3;
#define ERROR_WINSOCK_SOCKET_SEND_FAILURE       4;
#define ERROR_WINSOCK_SOCKET_BIND_FAILURE       5;

struct sockaddr_in addrServer;    // IN_ADDR holds an IPv4 address

#define MSGSENDLENGTH 100		   
CHAR message[MSGSENDLENGTH];
//NOTE - If your project is using Unicode instead of ASCII, you will probably need to change CHAR to WCHAR

CHAR * ipv4addr;

#define MSGRECVLENGTH 100		   
CHAR msgRecv[MSGRECVLENGTH];       // array to hold received messages
//NOTE - If your project is using Unicode instead of ASCII, you will probably need to change CHAR to WCHAR

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
// no need to change anything from here...
	// Before using Winsock calls on Windows, the Winsock library needs to be initialized...
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Winsock error Code : %d", WSAGetLastError());
		return ERROR_WINSOCK_INIT_FAILURE;
	}

	/* The socket() call takes three arguments. The first is the network protocol "Address Family", hence the AF_prefix.
	The two most common are AF_INET for IPv4 and AF_INET6 for IPv6. The next asks for the port type, which is usually a
	TCP port with SOCK_STREAM, or a UDP port with SOCK_DGRAM. The third parameter is the specific protocol, such as ICMP,
	IGMP, or for the purposes of the program, TCP, which uses the constant IPPROTO_TCP. */
	
	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return ERROR_WINSOCK_SOCKET_CREATE_FAILURE;
	}
	// The socket now exists...but it isn't configured with an IP address or port number yet.

	ipv4addr = ipaddr;  
	InetPton(AF_INET, ipv4addr , &addrServer.sin_addr.s_addr);  // converts the IP address as a text string in ipv4addr to a number
	addrServer.sin_family = AF_INET;							// Must agree with the socket Address Family type
// ... to here
	addrServer.sin_port = htons(60481);							// htons() converts the host endianness to network endianness
																// This should always be used when transmitting integers
																// ntohs() converts the opposite way for receiving integers.
	//CE-480 The socket port number above will need to be changed to match the port that the server is listening on. 
	// You do NOT specify anything about the client's port number. The OS will pick one for you.
	{
		InetNtop(AF_INET, &(addrServer.sin_addr), (PSTR)ipv4addr, INET_ADDRSTRLEN);   // converts the IP address as a 32-bit number to a printable character array
		printf("Attempting to connect to %s on port %d...\n", ipv4addr, ntohs(addrServer.sin_port));
	}
	//Connect to remote server
	if (connect(s, (struct sockaddr *) &addrServer, sizeof(addrServer)) < 0)   // negative return values indicate an error. For this demo/project, we don't need to troubleshoot, so just "scream and die" 
	{
		printf("Could not connect socket");
		return ERROR_WINSOCK_SOCKET_CONNECT_FAILURE;
	}
	
	// At this point, the connection is established. The rest of the code is customized depending on the order of messages. It needs to be written carefully
	// to correspond to what the server expects. For example, the server starts be sending the number of the connection, so the server starts with a send(), and the client starts with a recv().
	
	int bytesRecv, mynumber;
	bytesRecv = recv(s, msgRecv, MSGRECVLENGTH, 0);  // blocks here until something is received on the socket.
	
	if (bytesRecv > 0) {
		printf("%s\n", msgRecv);
	}

	sscanf_s(msgRecv, "You are connection %d", &mynumber); // Shows how to use the scanf for a buffer to pick out a number
	// ...recalling that for a scan, you must supply the ADDRESS of where to store the value
	printf("I was just told that I'm number %d\n", mynumber);

	printf("Going to sleep for 5 seconds to show that the server will block on a recv() call...\n");
	Sleep(5000);

	char message[80];
	printf("Enter your message:");
	gets(message); // assumes that the user types 79 characters or less.
	
	if (send(s, message, (int)strlen(message), 0) < 0) // negative return values indicate an error. For this demo/project, we don't need to troubleshoot, so just "scream and die" 
	{
		printf("Error sending message");
		return ERROR_WINSOCK_SOCKET_SEND_FAILURE;
	}

	// In more complex interactions, the client may loop. This simple demo just exits after one message.

	printf("Terminating...");
	Sleep(5000);
	closesocket(s);
	WSACleanup();
	return 0;
}