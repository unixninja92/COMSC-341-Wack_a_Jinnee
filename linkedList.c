#include "linkedList.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Creates a singly-linked list of JinneeBottles with the first item 
 * being bottle. 
 */
linkedList* createLL(JinneeBottle* bottle){
  linkedList* list;
  list = (linkedList *) malloc(sizeof(linkedList));
  list->first = (struct linkListNode*)malloc(sizeof(struct linkListNode));
  list->first->next = 0;
  list->first->bottle = bottle;
  return list;
}

/*
 * Adds bottle to the beginning of the linked list link
 */
int addItemLL(linkedList* link, JinneeBottle* bottle){
  struct linkListNode* newNode;
  newNode = (struct linkListNode*)malloc(sizeof(struct linkListNode));
  if(link->first != 0){
    newNode->next = link->first;
  }
  else{
    newNode->next = 0;
  }
  newNode->bottle = bottle;
  if(bottle != 0){
    link->first = newNode;
    return 1;
  }
  return 0;
}

/*
 * Removes bottle from the linked list link
 */
void removeItemLL(linkedList* link, JinneeBottle* bottle){
  struct linkListNode* current = link->first;
    if(current->bottle == bottle){
      if(current->next != 0)
	link->first = current->next;
      else{
	link->first = 0;
	return;
      }
    }
    while( current->next != 0 && current->next->bottle != bottle){
      current = current->next;
    }
    if(current->next != 0){
	if(current->next->next != 0)
	  current->next = current->next->next;
	else
	  current->next = 0;
    } 
  
}
