#include "chat.h"
/************************/
/** User Class Methods **/
/************************/
user::user(){ // setting to initial state ie. no relevant data.
  memset(this->name, 0, sizeof(this->name)); // initialize name to null
  for(int i = 0; i < MAX_ROOMS; ++i){
    this->chatNum[i] = -1; // assign all rooms to -1
    this->chats[i] = NULL;
  }
  this->sockIndex = -1;
  this->sockNum = -1;
  this->chatsCounter = 0;
}
user::user(int i, char nameArg[]){
}
user::~user(){ // set to NULL equivalent, and free dynamic memory (if any)
  memset(this->name, 0, sizeof(this->name)); // initialize name to null
  for(int i = 0; i < MAX_ROOMS; ++i){
    this->chatNum[i] = -1; // assign all rooms to -1
    this->chats[i] = NULL;
  }
  this->chatsCounter = 0;
  this->sockIndex = -1;
  this->sockNum = -1;
}
void user::deleteUser(int & userCount, class room & chatApp){
  chatApp.removeUser(*(this));
  memset(this->name, 0, sizeof(this->name)); // initialize name to null
  for(int i = 0; i < MAX_ROOMS; ++i){
    this->chatNum[i] = -1; // assign all rooms to -1
    this->chats[i] = NULL;
  }
  this->sockIndex = -1;
  this->sockNum = -1;
  this->chatsCounter = 0;
  --userCount; // decrement user list counter by 1.
}
void user::myInfo(char appMessage[]){
  char convertNum[25];
  char localMessage[BUFFER];
  memset(localMessage, 0, sizeof(localMessage)); // clear message
  strcpy(localMessage, "Name: ");
  strcat(localMessage, this->name);
  strcat(localMessage, "\n");
  strcat(localMessage, "Chats you're in: ");
  for(int i = 0; i < MAX_ROOMS; ++i){
    if(this->chatNum[i] != -1){
    }
  }
  for(int i = 0; i < this->chatsCounter; ++i){
    strcat(localMessage, this->chats[i]->name);
    strcat(localMessage, ", ");
  }
  strcpy(appMessage, localMessage); // oops forgot to copy..
  // maybe add a name lookup. . . 
}
void user::displayAllUsers(char appMessage[], class user connectedUsers[]){
  char localMessage[BUFFER];
  int i = 0;
  memset(localMessage, 0, sizeof(localMessage));
  strcpy(localMessage, "User connected: ");
  for(i = 0; i < MAX_USERS; ++i){
    if(connectedUsers[i].sockIndex != -1){
      strcat(localMessage, connectedUsers[i].name);
      strcat(localMessage, ", ");
    }
  }
  strcat(localMessage, "\n");
  strcpy(appMessage, localMessage);
}
int user::assignToRoom(int chatNum, room & chatApp){
  int i = 0;
  int toReturn = FALSE;
  if(chatNum > MAX_ROOMS-1 || chatNum < 0){
    return toReturn;
  }
  user * temp = NULL;
  cout << "Assigned a user to room: " << chatNum << endl;
  for(i = 0; i < MAX_ROOMS; ++i){
    if(this->chatNum[i] == -1){
      this->chatNum[i] = chatNum; // assign to requested room
      break;
    }
  }
  if(i < MAX_ROOMS){
    this->chats[this->chatsCounter] = &chatApp.chatRooms[chatNum];
    ++this->chatsCounter;
    int index = chatApp.chatRooms[chatNum].users; // assign index to a smaller variable for readability.
    ++chatApp.chatRooms[chatNum].users; // increment the variable (doesn't matter what order this is done in, as we have index already).
    chatApp.chatRooms[chatNum].roomUsers[index] = this; // this room now has "this" user in it.
    // instead of manually deep copying data, I can use pointers.
    toReturn = GENERIC_SUCCESS;
  }

  return toReturn;
}
int user::retrieveSockIndex(){
  return this->sockIndex;
}
void user::addUser(int client_sock_index, int sockNum){
  int i = 0;
  char convertNum[25];  // used to convert number to string
  int randNum = rand() % 100003; // some arbitrary prime number
  strcpy(this->name, DEFAULT_NAME);
  sprintf(convertNum, "%d", randNum);
  strcat(this->name, convertNum);
  this->sockIndex = client_sock_index;
  this->sockNum = sockNum;
  for(i = 0; i < MAX_ROOMS; ++i){
    this->chatNum[i] = -1; // user initially is in no chat rooms except lobby
  } 
}
int user::sendMessage(int roomNum, char toSend[], room & chatApp){
//  cout << "Enter send" << endl;
  int success = FALSE;
  int i = 0;
  char localMessage[BUFFER];
  strcpy(localMessage, this->name);
  strcat(localMessage, " Says: ");
  strcat(localMessage, toSend);
  if(roomNum >= MAX_ROOMS){
    return success; // this will return a FAIL
  }
  int userCount = chatApp.chatRooms[roomNum].users; // grab number of users in this chat
  success = GENERIC_SUCCESS;
  for(i = 0; i < userCount; ++i){
    if(chatApp.chatRooms[roomNum].roomUsers[i] != NULL){
      send(chatApp.chatRooms[roomNum].roomUsers[i]->sockNum, localMessage, sizeof(localMessage), MSG_NOSIGNAL); 
    } // broadcasts message to all users that are within this room.
  }
  return success;
}
int user::leaveRoom(room & chatApp, int chatNum){
  int success = FALSE;
  int i = 0;
  for(i = 0; i < MAX_ROOMS; ++i){
    if(this->chatNum[i] == chatNum){ // if user is in fact in this chat room
      success = GENERIC_SUCCESS;
      this->chatNum[i] = -1; // remove room number from user's list
      --this->chatsCounter; // decrement user's room counter
      this->chats[this->chatsCounter] = NULL; // remove the room pointer from the user's list.
      chatApp.leaveChannel(*(this), chatNum);
      // NOTE: It's important that the counter is decremented FIRST, as the counter always indicates
      // index+1, so first decrement to have just the index, and then remove.
    }
  }
  return success;
}
/************************/
/** Room Class Methods **/
/************************/
room::room(){
  this->initializeRooms();
  this->roomCounter = 0;
  // roomUsers is already initialized by user class constructor.
}
room::~room(){
  delete chatRooms;
}
// Since my data structures are arrays, I need to re-factor arrays sometimes
// to account for drift inside of the arrays.
void room::remakeRoomUserArray(int index){
  user * tempUserArr[MAX_USERS];
  int tempCount = 0;
  int localCount = 0;
  for(int j = 0; j < MAX_USERS; ++j){
    tempUserArr[j] = NULL; // quickly initialize temp arr to NULL
  }
  int i = 0;
  for(i = 0; i < MAX_USERS; ++i){
    if(this->chatRooms[index].roomUsers[i] != NULL){ // first copy the current
      tempUserArr[localCount] = this->chatRooms[index].roomUsers[i];  // user list
      ++localCount;
      this->chatRooms[index].roomUsers[i] = NULL; // and then 'erase' from list
    }
  }
  for(i = 0; i < MAX_USERS; ++i){
    if(tempUserArr[i] != NULL){
      this->chatRooms[index].roomUsers[tempCount] = tempUserArr[i];
      ++tempCount; // increment to next index
    }
  }
}
int room::leaveChannel(user & toRemove, int index){ // Remove a user from ONE room. Used when requested.
  int i = 0;
  for(i = 0; i < MAX_USERS; ++i){
    // if a pointer exists in the pointer array that matches the address of the user,
    // then remove and return a success.
    if(this->chatRooms[index].roomUsers[i] == &toRemove){
      this->chatRooms[index].roomUsers[i] = NULL; // remove from list of users in this channel
      --this->chatRooms[index].users; // decrement user counter
      this->remakeRoomUserArray(index); // 're-stack' array to prevent rightward drift.
    }
  }
}
void room::removeUser(user & toRemove){ // Purge a user from ALL rooms. Used when a user disconnects from server.
  int i = 0;
  int j = 0;
  for(i = 0; i < MAX_ROOMS; ++i){
    if(toRemove.chatNum[i] != -1){
      // now search for the user
      for(j = 0; j < MAX_USERS; ++j){
        if(this->chatRooms[toRemove.chatNum[i]].roomUsers[j] == &toRemove){
          cout << "Room Num: " << toRemove.chatNum[i] << endl;
          cout << "User name: " << toRemove.name << endl;
          // if the address of our user is the same as the user pointer
          // then set that user to NULL.
          this->chatRooms[i].roomUsers[j] = NULL;
          --this->chatRooms[toRemove.chatNum[i]].users; // decrement user counter
          this->remakeRoomUserArray(toRemove.chatNum[i]);
        }
      }
    }
  } 
}
void room::initializeRooms(){ // initialize all data structure data
  int i = 0;
  int j = 0;
  this->chatRooms = new roomObj[MAX_ROOMS];
  for(i = 0; i < MAX_ROOMS; ++i){
    this->chatRooms[i].num = i;
    strcpy(this->chatRooms[i].name, "free");
    this->chatRooms[i].users = 0;
    for(j = 0; j < MAX_USERS; ++j){
      this->chatRooms[i].roomUsers[j] = NULL;
    }
  }
}
int room::addRoom(char name[]){
  int i = 0;
  int roomNum = 0;
  for(i = 0; i < MAX_ROOMS; ++i){
    if(strcmp(this->chatRooms[i].name, "free") == 0){
      strcpy(this->chatRooms[i].name, name);
      return i;
    }
  }
  if(i == MAX_ROOMS-1){
    cout << "Room could not be added!" << endl;
    roomNum = -1;
  }
//  return roomNumg
  return GENERIC_SUCCESS;
}
void room::displayRoom(char message[]){
  cout << "Display" << endl;
  int i = 0;
  char temp[BUFFER];
  char convertNum[25]; // Simply used to perform conversion from ints to chars 
  memset(temp, 0, sizeof(temp)); // fill with null string
  strcpy(temp, "Current chat rooms: \n");
  for(i = 0; i < MAX_ROOMS; ++i){
    strcat(temp, "Room no. ");
    sprintf(convertNum, "%d", i);
    strcat(temp, convertNum);
    strcat(temp, " || Name/Status: ");
    strcat(temp, this->chatRooms[i].name);
    strcat(temp, " || Users Present: ");
    sprintf(convertNum, "%d", this->chatRooms[i].users);
    strcat(temp, convertNum);
    strcat(temp, "\n ");
  }
  cout << "Message length is: " << strlen(temp) << endl;
  strcpy(message, temp); // copy constructed message into passed in char array
}
int room::deleteRoom(char toDelete[]){
  int i = 0;
  int success = FALSE;
  for(i = 0; i < MAX_ROOMS; ++i){
    if(strcmp(toDelete, this->chatRooms[i].name) == 0){
      strcpy(this->chatRooms[i].name, "free"); // 'delete room'
      success = GENERIC_SUCCESS;
      // ideally remove users from that room at this point . .
      break;
    }
  }
  return success;
}
/*********************/
/** Misc. Functions **/
/*********************/
void parseName(char recvMessage[], char name[]){
  char ch;
  char localMessage[BUFFER];
  int localCount = 0;
  int flag = -1;
  memset(localMessage, 0, sizeof(localMessage));
  int i = 1;
  if(isalnum(recvMessage[0])){
    return;
  }
  ch = recvMessage[i];
  while(!isspace(ch)){
    ++i;
    ch = recvMessage[i];
  }
  // by the end of this I will have a command, therefore I increment 1 more space
  ++i;
  ch = recvMessage[i]; // reassign char to first character in the name
  while(isalnum(ch) && ch != '\n' && ch != '\0'){
    localMessage[localCount] = ch; // construct name piece by piece until ch is no longer an alphanumeric
    ++localCount;
    ++i;
    ch = recvMessage[i];
  }
  strcpy(name, localMessage);
}
int parseNumber(char recvMessage[], char toReturn[], int opCode){
  int chatNum = 0;
  char temp[BUFFER];
  char * ptr = NULL;
  char * termCheck = NULL;
  char ch;
  parseName(recvMessage, temp);
  chatNum = atoi(temp);
  if(opCode == RETURN_MESSAGE){ // the caller function has requested I return the remainder of the message.
//    ptr = strchr(recvMessage, ch);
    if(chatNum >= 10){ // then I must search for the substring of the second digit
      ch = temp[1];
    }
    else{ // then I must search for the substring of the single digit
      ch = temp[0];
    }
    ptr = strchr(recvMessage, ch);
    termCheck = ptr + 1;
    ptr = ptr + 2; // adjust pointer position to ignore the number pointer, and to ignore the space character.
    if(termCheck[0] == '\0'){ // if the user just typed "#send <roomNum>" without anything else, then return an empty space character
      strcpy(toReturn, " ");
      cout << "null char detected" << endl;
    }
    else{
      strcpy(toReturn, ptr);
    }
    
  }
  return chatNum;
}
