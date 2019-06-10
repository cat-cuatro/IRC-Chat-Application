#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <time.h>

#define PORT_NUM 15764
#define FALSE -1
#define BUFFER 2048
#define MAX_USERS 20
#define MAX_NAME 25
#define MAX_ROOMS 20
#define SEND_DATA 2
#define GENERIC_SUCCESS 1
#define DEFAULT_NAME "anonymous"
#define IGNORE_MESSAGE 1
#define RETURN_MESSAGE 2
using namespace std;


class user{
  public:
    user();
    user(int i, char nameArg[]);
    ~user();
    // functions
    void greet();
    void deleteUser(int & userCount, class room & chatApp);
    void addUser(int client_sock_index, int sockNum);
    int assignToRoom(int chatNum, class room & chatApp);
    int retrieveSockIndex();
    void myInfo(char appMessage[]);
    void displayAllUsers(char appMessage[], class user connectedUser[]);
    int leaveRoom(room & chatApp, int chatNum);
    int sendMessage(int roomNum, char toSend[], room & chatApp);
    // variables
    char name[MAX_NAME]; // the name of the users
    int chatNum[MAX_ROOMS]; // the unordered array of room numbers that a user is a part of
    int sockIndex; // the socket index location of the user
    int sockNum; // the socket number of the user
    struct roomObj * chats[MAX_ROOMS]; // unordered array of pointers that point to chats a user is a part of
    int chatsCounter;  // a head count of the number of rooms a user is a part of.
  
};
class room{
  public:
    room();
    ~room();
    //functions
    int addRoom(char name[]);
    void displayRoom(char message[]);
    void removeUser(user & toRemove); // removes user from ALL channels
    int leaveChannel(user & toRemove, int index); // removes user from specified channel
    int deleteRoom(char toDelete[]);
    void initializeRooms();
    void remakeRoomUserArray(int index);

    
    //variables
    struct roomObj * chatRooms; // dynamically allocated number of chat rooms
    int roomCounter; // a headcount of the number of chat rooms that exist
};
struct roomObj{
  int num; // the room's number
  char name[MAX_NAME]; // the name of the room
  user * roomUsers[MAX_USERS]; // the users in the room
  int users; // the headcount of users in the room
};
/********************/
/** Misc Functions **/
/********************/
int screenInput(char recvMessage[]);
void commandsList(char appMessage[]);
int executeCommand(int flag, char appMessage[], room & chatApplication, char recvMessage[], user connectedUser[], int userIndex);
void parseName(char message[], char name[]);
int parseNumber(char message[], char toReturn[], int opCode);
