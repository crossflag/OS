/*******************************************************************************
*   course:       CS 4328.253 Spring 2020
*   author:       Diego Santana Pelayo & Mirna Elizondo
*   due:          03/09/2020
*   Pair War is a simple card game with one dealer, 3 players,
*   and a single deck of cards. The game is composed of three rounds.
*******************************************************************************/
//libraries
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
//Main definition of threads, rounds, and cards. Log file also instantiated
#define NUM_THREADS 3
#define NUM_OF_CARDS 52
#define LOG_FILE "log.txt"
#define NUM_OF_ROUNDS 3

//Inifialization of global variables in order for pthreads to take args and variables.
FILE * logFile;
int deck[NUM_OF_CARDS];
int *top, *bottom;
int seed;
bool gameOver = false;
int turn = 0;
int round_num = 1;
//Hand is built as a struct in order to have simpler implementation.
struct hand{
  int card1, card2;
};
hand hand1, hand2, hand3;
//Pthreads conditions and mutexes intialized
pthread_mutex_t mutex_turn_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t win_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_dealer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t dealerThread;
pthread_t playerThread[NUM_THREADS];

//Prototype functions created and set to voids
void setUpGame();
void shuffle();
void takeTurn(long, hand);
void *players_thr(void *p_id);
void *dealer_thr(void *arg);
void initiate_round();
void deal();
void displayDeck();
void displayDeckAtBeginning();
//Main function takes in seed argument from command line and sets up game
int main (int argc, char *argv[]) {
  logFile = fopen("log.txt", "w");
  seed = atoi(argv[1]);
  srand(seed);
  setUpGame();
//Rounds taken and initiated until condition is satisfied.
  while(round_num <= NUM_OF_ROUNDS){
    initiate_round();
    round_num++;
    gameOver = false;
  }
  //Creates logfile and exits program. Log file also created in designated program folder.
  fclose(logFile);
  exit(EXIT_SUCCESS);
}
//Base function to set up the deck and cards to assignment specifications
void setUpGame(){
  int cardNum = 0;
  int deckNum = 0;
  int suit = 4;
  int temp  = NUM_OF_CARDS;
  while ((deckNum < temp) && (cardNum < (temp/suit))){
    for( int i = 0; i < suit; i++){
      deck[deckNum] = cardNum;
      deckNum++;
    }
    cardNum++;
  }
  top = deck;
  bottom = deck + (temp - 1);
}
//Round intialized and threads all created and joined in this function.
void initiate_round(){
    printf("Round: %d \n\n", round_num);
    fprintf(logFile, "Round: %d \n\n", round_num);
    displayDeckAtBeginning();

    int deal_tmp = pthread_create(&dealerThread, NULL, &dealer_thr, NULL);

    int play_tmp;
    for(long i = 1; i <= NUM_THREADS; i++){
      play_tmp = pthread_create(&playerThread[i], NULL, &players_thr, (void *)i);
    }
    pthread_join(dealerThread, NULL);
    for(int j = 0; j < 3; j++){
      pthread_join(playerThread[j], NULL);
    }
    fprintf(logFile, "DEALER: shuffle\n"); shuffle();
}
//Player threads defined and checks instantiated, Iterates through players and hands.
//Assigns players to hands and forces turns while initiating mutex locks.
void *players_thr(void *p_id){
  long playerid = (long)p_id;
  hand curhand;
  if( round_num == 1){
    if (playerid == 1){
        curhand = hand1;
    }
    else if(playerid == 2){
        curhand = hand2;
    }
    else if(playerid == 3){
        curhand = hand3;
    }
  }
  else if(round_num == 2){
    if (playerid == 2){
        curhand = hand1;
    }
    else if(playerid == 3){
        curhand = hand2;
    }
    else if(playerid == 1){
        curhand = hand3;
    }
  }
  else if( round_num == 3){
    if (playerid == 3){
        curhand = hand1;
    }
    else if(playerid == 1){
        curhand = hand2;
    }
    else if(playerid == 2){
        curhand = hand3;
    }
  }

  while(gameOver == 0){
    pthread_mutex_lock(&mutex_turn_lock);
      while (playerid != turn && gameOver == 0){
        pthread_cond_wait(&condition, &mutex_turn_lock);
      }
      if(gameOver == 0){
        takeTurn(playerid, curhand);
      }
    pthread_mutex_unlock(&mutex_turn_lock);
  }
  fprintf(logFile, "PLAYER %ld: exits round \n", playerid);
  pthread_exit(NULL);
  }
//Inifitates the dealer as player zero. Creates mutex locks for dealer.
void *dealer_thr(void *arg){
  long player = 0;
  turn = 0;
  hand dealhand;
  takeTurn(player, dealhand);

  pthread_mutex_lock(&mutex_dealer_lock);
    while(!gameOver){
        pthread_cond_wait(&win_condition, &mutex_dealer_lock);
      }
  pthread_mutex_unlock(&mutex_dealer_lock);
  pthread_exit(NULL);
}
//Initializes a turn for player after checking player ID.
//If player is zero dealer thread takes turn.
//Checks winners or losers and pushes conditionals forward
void takeTurn(long player, hand curhand){
  if(player == 0){
    shuffle();
    deal();
  }else{
    printf("PLAYER %ld: \n", player);
    fprintf(logFile, "PLAYER %ld: hand %d\n", player, curhand.card1);
    curhand.card2 = *top;
    top = top + 1;
    fprintf(logFile, "PLAYER %ld: draws %d \n", player, curhand.card2);


    if(curhand.card1 == curhand.card2){
      printf("HAND: %d %d\n", curhand.card1 ,curhand.card2);
      printf("WIN! yes\n");
      fprintf(logFile, "PLAYER %ld: wins\n", player);
      gameOver = true;
      pthread_cond_signal(&win_condition);
      }
      else {
        printf("HAND: %d\n", curhand.card1);
        printf("WIN! no\n");
        top--;
        int *ptr = top;
        while(ptr != bottom){
          *ptr = *(ptr+1);
          ptr++;
        }
        int i = rand() % 2;
        if(i == 0){
          fprintf(logFile, "PLAYER %ld: discards %d \n", player, curhand.card1);
          *bottom = curhand.card1;
          curhand.card1 = curhand.card2;
          fprintf(logFile, "PLAYER %ld: hand %d\n", player, curhand.card1);
        }else{
          fprintf(logFile, "PLAYER %ld: discards %d \n", player, curhand.card2);
          *bottom = curhand.card2;
          fprintf(logFile, "PLAYER %ld: hand %d\n", player, curhand.card1);
        }
        displayDeck();
    }
  }
  turn++;
  if (turn > NUM_THREADS) turn = 1;
  pthread_cond_broadcast(&condition);
}
//Displays the deck.
void displayDeck(){
  fprintf(logFile, "Deck: ");
  int *p = top;
  while(p != bottom){
    fprintf(logFile, "%d ", *p);
    p++;
    if(p == bottom){
      fprintf(logFile, "%d \n", *p);
    }
  }
}
//Displays the deck for the beginning of every round on the terminal
void displayDeckAtBeginning(){
  printf("Deck: ");
  int *p = top;
  while(p != bottom){
    printf("%d ", *p);
    p++;
    if(p == bottom){
      printf("%d \n", *p);
    }
  }
}
//Deals a card to each players hand.
void deal(){
  hand1.card1 = *top;
  top++;
  hand2.card1 = *top;
  top++;
  hand3.card1 = *top;
  top++;
}
//Randomizes the deck based on the seed argv.
void shuffle(){
  for(int i = 0; i < (NUM_OF_CARDS - 1); i++){
    int position = i + (rand() % (NUM_OF_CARDS -i));
    int temp = deck[i];
    deck[i] = deck[position];
    deck[position] = temp;
  }
}
