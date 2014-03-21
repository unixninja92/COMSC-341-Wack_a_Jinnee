#ifndef STRUCTS_H
#define STRUCTS_H

#include <X11/Xlib.h>

/*
 *========================================================
 * Data structure for items in a list. Cheesy, but easy.
 *========================================================
 */
typedef struct {
  GC gc;
          int type;
          int x, y;
          XPoint *points; int npoints;//Data for lines
          char string[100]; // Data for strings
          int length;
          int width, height; // Data for rectangles
  int startAngle, endAngle; // Data for ellipses
        } Item;

/*
 *========================================================
 * Data structure for a list
 *========================================================
 */
typedef struct list {
          Item *item;
          struct list *next;
          } List, *ListPtr;
 
/*
 *========================================================
 * Data structure for keeping track of everything about
 * each bottle including the bottle, the cork, the jinnee,
 * and if it is moving or has been closed. 
 *========================================================
 */

typedef struct {
  int id;
  Item *bottle;
  Item *cork;
  Item *jinnee;
  int x;
  int y;
  int width_modifier;
  int height;
  int moving;
  int closed;
} JinneeBottle;

#endif
