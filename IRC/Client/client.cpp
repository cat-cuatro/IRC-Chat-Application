#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;
#define PORT_NUM 15764
#define FALSE -1
#define BUFFER 2048


int main(){
  int net_socket = 0; // Must first create our socket
  int success = 0;
  char message[BUFFER] = {'\0'};
  net_socket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET; // sets family of address so we know what address we're using
  server_address.sin_port = htons(PORT_NUM); // htons() correctly converts integer into understandable data by server
  server_address.sin_addr.s_addr = INADDR_ANY;
  
  int connectSuccessful = connect(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  if(connectSuccessful == FALSE){
    cout << "Connection failed!" << endl;
    exit(0);
  }
  else{
    recv(net_socket, (char*)&message, sizeof(message), 0);
    cout << message << endl;
  }
  while(true){
    cout << ">";
    cin.get(message, BUFFER, '\n');
    cin.ignore(BUFFER, '\n');
    if(strcmp(message, "exit") == 0){
      send(net_socket, (char*)&message, sizeof(message), 0);
      break;
    }
    success = send(net_socket, (char*)&message, sizeof(message), 0);
    read(net_socket, (char*)&message, sizeof(message));
    if(strcmp(message, "exit") == 0 || success == -1){
      cout << "Server quit." << endl;
      break;
    }
    cout << ">>> " << message << endl;
  }
  
  // once done, close the socket.
  close(net_socket);
  return 0;
}
