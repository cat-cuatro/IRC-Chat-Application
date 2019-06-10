#include "chat.h"
int main(){
  // USER/CHATROOM DATA //
  user connectedUser[MAX_USERS];
  room chatApplication;
  int userCounter = 0;
  int flag = 0;
  srand(0); // seed time
  //***********/

  char message[BUFFER];
  char receivedMessage[BUFFER];
  char appMessage[BUFFER];
  char welcome[BUFFER] = "Welcome to the server!";
  
  // create our server socket
/* For multiple users config */
  int server_socket;
  int addressLength;
  int new_socket;
  int client_sock[MAX_USERS];
  int activity;
  int i;
  int valread;
  int sd;
  int max_sd;
  int opt = true;
/*****************************/
  fd_set readfds; // socket descriptors
  for(i = 0; i < MAX_USERS; ++i){
    client_sock[i] = 0; // initialize socket array to 0.
  } 

  // we need client sockets.
  int client_socket = 0;
  int clientsConnected = 0;
  int successful = 0;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == FALSE){
    cout << "Server failed to create socket!" << endl;
    exit(0);
  }
  //define server address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT_NUM);
  server_address.sin_addr.s_addr = INADDR_ANY;
  successful = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
  if(successful == FALSE){
    cout << "Failed to set socket opt!" << endl;
    exit(0);
  }
  // bind our socket to the specified IP and port
  successful = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
  if(successful == FALSE){
    cout << "Server failed to bind!" << endl;
    exit(0);
  }
  successful = listen(server_socket, MAX_USERS); // first argument is the server socket. Second arg is max # requests
  if(successful == FALSE){
    cout << "Server failed to listen!" << endl;
    exit(0);
  }
  addressLength = sizeof(server_address);
  cout << "Wait for a connection . . " << endl;
  while(true){
    FD_ZERO(&readfds); // clear/init sockets
    FD_SET(server_socket, &readfds);
    max_sd = server_socket;
    for(i = 0; i < MAX_USERS; ++i){
      sd = client_sock[i]; // Search for the maximum descriptor, and add to the socket set
      if(sd > 0){
        FD_SET(sd, &readfds); // read-only
      }
      if(sd > max_sd){ // basically a counter to use for select()
        max_sd = sd;
      }
    }
    activity = select(max_sd+1, &readfds, NULL, NULL, NULL);
    if((activity < 0) && (errno!=EINTR)){
      cout << "Error with select!" << endl;
    }
    if(FD_ISSET(server_socket, &readfds)){
      if((new_socket = accept(server_socket, (struct sockaddr*)&server_address, (socklen_t*)&addressLength)) < 0){
        cout << "Problem with accept!" << endl;
        exit(0);
      }
      cout << "New connection to server: " << endl;
      cout << "FD: " << new_socket;
      cout << " IP: " << inet_ntoa(server_address.sin_addr); // revisit this.
      cout << " PORT: " << ntohs(server_address.sin_port) << endl;
      
      send(new_socket, welcome, sizeof(welcome), 0);

      for(i = 0; i < MAX_USERS; ++i){ // add new socket to socket array
        if(client_sock[i] == 0){
          client_sock[i] = new_socket;
          cout << "Adding to list of sockets as: " << i << endl;
          // add new user to user database
          connectedUser[i].addUser(i, new_socket);
          break;
        }
      }
    }
    // otherwise, it is an IO operation for another socket.
    for(i = 0; i < MAX_USERS; ++i){
      sd = client_sock[i];
      if(FD_ISSET(sd, &readfds)){
        // Check if for being closed, ans read incoming message
        valread = read(sd, message, BUFFER);
        cout << "Valread: " << valread << endl;
        cout << "length: " << strlen(message) << endl;
        if(valread == 0 || valread == -1 || strcmp(message, "exit") == 0){
          // a client disconnected, grab credentials and print
          getpeername(sd, (struct sockaddr*)&server_address, (socklen_t*)&addressLength);
          cout << "Client disconnected: " << endl;
          cout << "IP: " << inet_ntoa(server_address.sin_addr);
          cout << " PORT: " << ntohs(server_address.sin_port);
          cout << endl;
          close(sd); // close this user's port
          client_sock[i] = 0;
          connectedUser[i].deleteUser(userCounter, chatApplication);
        }
        else{
          flag = screenInput(message);
          cout << "User message: " << message << endl;
          if(flag > 0){
            successful = executeCommand(flag, appMessage, chatApplication, message, connectedUser, i);
            if(successful == SEND_DATA){
              send(sd, appMessage, sizeof(appMessage), MSG_NOSIGNAL);
            }
          }
          successful = send(sd, message, sizeof(message), MSG_NOSIGNAL); // stops send method from flagging SIGPIPE
          if(successful == FALSE){
            cout << "Failed to send the message!" << endl;
          }
        }
      }
    }
  }
  // the first parameter is the structure, the second and third arg would contain the address of the client parameter
  // these find out where the client is connecting from and fills the data of the structure provided
  close(server_socket);

  return 0;
}
int screenInput(char recvMessage[]){
  char ch;
  char localMessage[BUFFER];
  int flag = -1;
  memset(localMessage, 0, sizeof(localMessage)); // null terminate my array before strcat.
  int i = 1;
  if(isalpha(recvMessage[0])){
    return flag;
  }
  ch = recvMessage[i]; // grab first non-symbol character from input.
  while(!isspace(ch)){
    localMessage[i-1] = ch;
    ++i; // increment index
    ch = recvMessage[i]; // examine value of next character.
  }
  // by the end, I will have something that is presumably a command.
  // therefore, I run the message through a series of checks.
  // if I have a matched function call, then I return an integer
  // that corresponds with some type of action the user desires.
  // I implement it this way to keep my program completely separate from the server code.
  if(strcmp(localMessage, "list") == 0){
    cout << "User requested command list." << endl;
    flag = 1;
  }
  else if(strcmp(localMessage, "crt") == 0){
    cout << "User requested create channel" << endl;
    flag = 2;
  }
  else if(strcmp(localMessage, "dlt") == 0){
    cout << "User request delete channel." << endl;
    flag = 3;
  }
  else if(strcmp(localMessage, "join") == 0){
    cout << "User requested join." << endl;
    flag = 4;
  }
  else if(strcmp(localMessage, "dsp") == 0){
    cout << "User requested to display all rooms." << endl;
    flag = 5;
  }
  else if(strcmp(localMessage, "dspu") == 0){
    cout << "User requests to display all users" << endl;
    flag = 6;
  }
  else if(strcmp(localMessage, "dspme") == 0){
    cout << "User requested to display their information." << endl;
    flag = 7;
  }
  else if(strcmp(localMessage, "send") == 0){
    cout << "User requested to send a msg to a channel." << endl;
    flag = 8;
  }
  else if(strcmp(localMessage, "leave") == 0){
    cout << "User requested to leave a channel." << endl;
    flag = 9;
  }
  else{
    cout << "User entered false command: " << localMessage << endl;
    flag = -1;
  }
  cout << "End of block" << endl;
  return flag;
}
int executeCommand(int flag, char appMessage[], room & chatApplication, char recvMessage[], user connectedUser[], int userIndex){
  int success = FALSE; // return code of 1 == generic success, 2 == send data to user
  char temp[BUFFER];
  int roomNum = 0;
  cout << endl;
  // There's no logical ordering here. They are ordered in the order I implement the methods.
  switch(flag){
    case 1: // List all commands
      commandsList(appMessage);
      success = SEND_DATA;
      break;
    case 2: // Create a chat room
      parseName(recvMessage, temp); // retrieves name from received string, and stores in temp
      success = chatApplication.addRoom(temp); // temp holds the name of the room to add
      if(success == FALSE){
        strcpy(appMessage, "Couldn't add room.\n");
        success = SEND_DATA;
      }
      else{
        success = GENERIC_SUCCESS;
      }
      break;
    case 3: // Delete a chat room
      parseName(recvMessage, temp);
      success = chatApplication.deleteRoom(temp);
      if(success == FALSE){
        strcpy(appMessage, "Couldn't remove that room.\n");
        success = SEND_DATA;
      }
      else{
        success = GENERIC_SUCCESS;
      }
      break;
    case 4: // Join a chat room
      roomNum = parseNumber(recvMessage, temp, IGNORE_MESSAGE); // retrieve room number from command string
      success = connectedUser[userIndex].assignToRoom(roomNum, chatApplication);
      if(success == FALSE){
        strcpy(appMessage, "Couldn't add you to that room.");
        success = SEND_DATA;
      }
      else{
        success = GENERIC_SUCCESS;
      }
      break;
    case 5: // Display all chat rooms
      cout << "Smashing??????" << endl;
      chatApplication.displayRoom(appMessage);
      success = SEND_DATA;
      break;
    case 6: // Display all users
      // user index not really in use here, just accessing member method
      connectedUser[userIndex].displayAllUsers(appMessage, connectedUser);
      success = SEND_DATA;
      break;
    case 7: // Display specific user's info
      connectedUser[userIndex].myInfo(appMessage);
      success = SEND_DATA;
      break;
    case 8: // Send a message to a specific chatroom
      roomNum = parseNumber(recvMessage, temp, RETURN_MESSAGE);
      success = connectedUser[userIndex].sendMessage(roomNum, temp, chatApplication);
      if(success == FALSE){
        strcpy(appMessage, "Couldn't send message!\n");
        success = SEND_DATA;
      }
      else{
        success = GENERIC_SUCCESS; // indicated as a 'generic success' because the data is sent
      }
      break;                     // within the function call itself
    case 9: // Leave a chat room. 
      roomNum = parseNumber(recvMessage, temp, IGNORE_MESSAGE);
      success = connectedUser[userIndex].leaveRoom(chatApplication, roomNum);
      if(success == FALSE){
        strcpy(appMessage, "Couldn't leave that chat room\n");
        success = SEND_DATA;
      }
      else{
        success = GENERIC_SUCCESS;
      }
      break;
    default: 
      // noOp on bad input
      break;
  }
  return success;
}
void commandsList(char appMessage[]){
  char localMessage[BUFFER];
  memset(localMessage, 0, sizeof(localMessage)); // clear buffer
  strcpy(localMessage, "*****Welcome to the server!*****\n");
  strcat(localMessage, "Command List:\n");
  strcat(localMessage, "1. List commands (#list)\n");
  strcat(localMessage, "2. Create a chatroom (#crt <name>)\n");
  strcat(localMessage, "3. Delete a chatroom (#dlt <name>)\n");
  strcat(localMessage, "4. Join a chatroom (#join <roomNum>)\n");
  strcat(localMessage, "5. Display all rooms (#dsp)\n");
  strcat(localMessage, "6. Display all users (#dspu)\n");
  strcat(localMessage, "7. Display your user info (#dspme)\n");
  strcat(localMessage, "8. Send a message (#send <roomNum> <message>\n");
  strcat(localMessage, "9. Leave a chat room (#leave <roomNum>)\n");
  strcpy(appMessage, localMessage);
}
