/*************************************************************
* CS 4328.253 Spring 2020
* Programming Assignment #2
* Due 4/24/20
* Diego Santana & Mirna Elizondo
* Virtual-memory page replacement algorithms
**************************************************************/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <bits/stdc++.h>

using namespace std;

int pageRange;// 0 <= 49
int frameNum; // 1 <= 30
int faultCount = 0;
int foundCount = 0;
string sequence;
int size = 100;
//the operating system keeps track of all pages in the memory in a queue,
// the oldest page is in the front of the queue. When a page needs to be
//replaced page in the front of the queue is selected for removal. If the pages
//is already in the queue it is a hit if not it is a miss/page fault
int FIFO(int pages[], int frameNum){
  int miss = 0;
  int hit = 0;
  double ratio = (double)hit / (hit+miss);

  unordered_set<int> set;
  queue<int> indx;

  for(int i=0; i < 100; i++){
    if(set.size() < frameNum){
      if(set.find(pages[i])==set.end()){
        set.insert(pages[i]);
        miss++;
        indx.push(pages[i]);
      }else{
        if(set.find(pages[i])==set.end()){
          int val = indx.front();
          indx.pop();
          set.erase(val);
          set.insert(pages[i]);
          miss++;
        }
      }
    }
  }

  hit = 100 - miss;

  cout<<"***************FIFO****************" << endl;
  cout<<"* hits: "<< hit << endl;
  cout<<"* misses: "<< miss << endl;
  cout<<"* ratio: "<< ratio << endl<< endl;

  return hit, miss;
}

//page least used gets replaced
int LRU(int pages[], int frameNum){
  int miss = 0;
  int hit = 0;
  double ratio = (double)hit / (hit+miss);

  unordered_set<int> s;
  unordered_map<int, int> indx;

  for (int i = 0; i < 100; i++){
    if (s.size() < (frameNum)){
      if(s.find(pages[i])==s.end()){
        s.insert(pages[i]);
        miss++;
      }
      indx[pages[i]]=i;
    }
    else{
      if(s.find(pages[i]) == s.end()){
        int lru = INT_MAX, val;
        for(auto it=s.begin(); it!=s.end(); it++){
          if (indx[*it]<lru){
            lru = indx[*it];
            val = *it;
          }
        }
        s.erase(val);
        s.insert(pages[i]);
        miss++;
      }
      indx[pages[i]] = i;
    }
  }
  hit = 100 - miss;
  cout<<"***************LRU****************" << endl;
  cout<<"* hits: "<< hit << endl;
  cout<<"* misses: "<< miss << endl;
  cout<<"* ratio: "<< ratio << endl<< endl;

  return hit, miss;
}

//In this algorithm, pages are replaced which would not be used for the
//longest duration of time in the future.

bool finder(int key, vector<int>& frame){
  for( int i = 0; i < frame.size(); i++){
    if(frame[i] == key){
      return true;
    }else{
      return false;
    }
  }
}
int psychic(int pages[], vector<int>& frame, int indx){
  int res = -1;
  int farthest = indx;
  for (int i = 0; i < frame.size(); i++){
    int j;
    for(j = indx; j < 100; j++){
      if (frame[j] == pages[j]){
        if (j > farthest){
          farthest = j;
          res = i;
        }
        break;
      }
    }
    if(j = 100){
      return i;
    }
  }
  return(res == -1) ? 0 : res;
}

int OPT(int pages[], int frameNum){
  int miss = 0;
  int hit = 0;
  double ratio = (double)hit / (hit+miss);

  vector<int> frame;

  for(int i = 0; i < 100; i++){
    if(finder(pages[i], frame)){
      hit++;
      continue;
    }
    if(frame.size() < frameNum){
      frame.push_back(pages[i]);
    }else{
        int j = psychic(pages, frame, i+1);
        frame[j] = pages[i];
    }
  }
  miss = 100 - hit;

  cout<<"***************OPT****************" << endl;
  cout<<"* hits: "<< hit << endl;
  cout<<"* misses: "<< miss << endl;
  cout<<"* ratio: "<< ratio << endl<< endl;

  return hit, miss;
}



int main(){
  srand(time(NULL));
  pageRange = rand() % 49;// for string # range from 0 <= 49
  frameNum = rand() % 30 + 1;// 1 <= 30
  int pages[size];
  //sequence creation
  for(int i = 0; i < 100; i ++){
      int value = rand() % 49;
      sequence = sequence + " " + to_string(value);
      pages[i] = value;
    }
  cout << "Pages: ";
  for (int i = 0; i < 100; i++){
    cout << pages[i]<< " ";
  }

  cout<<endl<< "Sequence:" << sequence << endl;
  cout<< "Number of frames: " << frameNum << endl;
  cout<< "Starting First-in First-out (FIFO) algorithm"<< endl<< endl;
  FIFO(pages, frameNum); //first-in first-out
  cout<< "Starting least recently used  (LRU) algorithm" << endl << endl;
  LRU(pages, frameNum); //least recently used
  cout<< "Starting optimal (OPT) algorithm " << endl << endl;
  OPT(pages, frameNum); //optimal
}
