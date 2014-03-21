#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "structs.h"

struct linkListNode{
  struct linkListNode* next;
  JinneeBottle* bottle;
};

typedef struct {
  struct linkListNode* first;
}linkedList;

linkedList* createLL(JinneeBottle*);
int addItemLL(linkedList*, JinneeBottle*);
void removeItemLL(linkedList*, JinneeBottle*);

#endif 
