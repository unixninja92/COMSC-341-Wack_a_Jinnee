/*
 * Basic drawing of circles, rectangeles, and strings was written by Elodie Fourquet
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/*=================================================
 * Header files for X functions
 ==================================================*/
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include "linkedList.h"

/*=================================================
 * Defines for item types
 ================================================= */
#define String 0
#define Rectangle 1
#define Lines 2
#define Ellipse 3


/*
 * Defines for window geometry and keyboard events
 */
#define Border 5
#define BufferSize 10

//defines the number of bottles the crate will contain
#define NumBottles 25


//defines the lenght between repaints/frames
#define SleepTime 150000

//defines how long till the next jinnee starts moving
#define JinneeGap 2000000

/*=================================================
 * defines how far the cork can travel before the jinnee
 * is considered escaped
 *=================================================
 */
#define CorkOffset 25

/*====================================================
 *Define global variables
 *====================================================
*/
unsigned long background, foreground;
int bottle = 0;
int splash = 1;
int end = 1;


/*
 *===================================================
 * timing methods
 ===================================================*/
double getTime(){
  struct timeval times;
  gettimeofday(&times, NULL);
  //printf("%i\n", times.tv_sec*1000000+times.tv_usec); 
  return times.tv_sec*1000000+times.tv_usec;
}// based on code from http://www.ccplusplus.com/2011/11/gettimeofday-example.html
  


/*
 *========================================================
 * Function to put out a message on error exits.
 *========================================================
 */
void error( char *str ) {
  printf( "%s\n", str );
  exit(0);
}


/*
 *======================================================
 * Function to repaint a display list
 *======================================================
 */
void repaint( ListPtr head, Display *display, Window window, GC gc ) {
  Item *it;
  int x, y;
  
  XClearWindow( display, window );
  while( head ) {
    it = head->item;
    x = it->x;
    y = it->y;

    switch( it->type ) {
    case String:
      XDrawImageString( display, window, it->gc, x, y, it->string, it->length );
      break;
    case Rectangle:
      XFillRectangle( display, window, it->gc, x, y, it->width, it->height );
      break;
    case Lines:
      XDrawLines(display, window, it->gc, it->points, it->npoints, 1);
      break;
    case Ellipse:
      XDrawArc(display, window, it->gc, x, y, it->width, it->height, it->startAngle*64, it->endAngle*64);
      break;
    default:
      error( "repaint: Unimplemented item.\n" ); 
      break;
    }
    head = head->next;
  }
  XFlush( display );
}

/*
 *======================================================
 * Insert an item at the head of the list.
 * This item will be drawn first so it will be at 
 * the back of the image.
 *======================================================
 */
ListPtr insertItem ( ListPtr head, Item *itp ) {
  ListPtr newh;  
  newh = ( ListPtr ) malloc( sizeof( List ) );
  newh->item = itp; newh->next = head;
  return newh;
}

/*
 *======================================================
 * Function to make a new String item; 
 * a rectangle item is similar.  Complete and test 
 * that function below
 *======================================================
 */
Item *mkStringItem( int x, int y, char *str, GC gc ) {
  Item *newh;
  newh = ( Item * ) malloc ( sizeof( Item ) );
  newh->gc = gc;
  newh->type = String; 
  newh->x = x; newh->y = y;
  strcpy( newh->string, str ); newh->length = strlen( str );
  return newh;
}


/*
*  Functions to make a line
*/
Item *mkLinesItem( XPoint *points, int npoints, GC gc) {
  Item *newh;
  newh = ( Item * ) malloc ( sizeof( Item ) );
  newh->type = Lines;
  newh->gc = gc;
  newh->points = points;
  newh->npoints = npoints;
  return newh;
}


/*
 *======================================================
 * Function to make a new Rectangle item
 *======================================================
 */
Item *mkRectangleItem( int x, int y, int width, int height, GC gc ) {
	Item *newh;
	newh = (Item *) malloc (sizeof( Item));
	newh->type = Rectangle;
	newh->gc = gc;
	newh->x = x;
	newh->y = y;
	newh->width = width;
	newh->height = height;
	return newh;
}

/*
 *======================================================
 * Function to make a new Ellipse item
 *======================================================
 */

Item *mkEllipseItem(int x, int y, int width, int height, int start, int end, GC gc){
  Item *newh;
  newh = (Item*) malloc(sizeof(Item));
  newh->type = Ellipse;
  newh->x = x;
  newh->y = y;
	newh->gc = gc;
	newh->width = width;
	newh->height = height;
	newh->startAngle = start;
	newh->endAngle = end;
	return newh;
}

/*
 *======================================================
 * Function to make a new Bottle item which is made up 
 * of a Lines Item
 *======================================================
 */
Item *mkBottle(int x, int y, GC gc) {
  Item * l;
  short height = 35, width = 35;
  XPoint *p = (XPoint * )malloc( sizeof(XPoint)*4);
  //printf("making bottles\n");
  p[0].x = (short)x+10;
  p[0].y = (short)y;
  p[1].x = 10;
  p[1].y = height;
  p[2].x = -width;
  p[2].y = 0;
  p[3].x = 10;
  p[3].y = -height;
  l = mkLinesItem(p, 4, gc);
  return l;
}

/*
 *======================================================
 * Function to make a new Cork item which is made up 
 * of a Rectangle Item
 *======================================================
 */
Item *mkCork(JinneeBottle*** bottles, int r, int c, GC gc){
  Item * cork;
  JinneeBottle* bottle = bottles[r][c];
  cork = mkRectangleItem( bottle->x-4, bottle->y, 13, 12, gc);
  bottle->cork = cork;
  return cork;
}

/*
 *======================================================
 * Function to make a new Jinnee item which is made up 
 * of a Ellipse Item
 *======================================================
 */
Item *mkJinnee(JinneeBottle*** bottles, int r, int c, GC gc){
  Item* jinnee;
  JinneeBottle* bottle = bottles[r][c];
  jinnee = mkEllipseItem(bottle->x-2, bottle->y + 18, 7, 7, 0, 360, gc);
  bottle->jinnee = jinnee;
  return jinnee;
}

/*
 *======================================================
 * Function to make a new Crate which contains a 2D array
 * of all the JinneeBottles as well as create all the 
 * bottles
 *======================================================
 */
ListPtr mkCrate(JinneeBottle*** bottles, ListPtr head, int rows, int columns, GC gc) {
  Item *bottle;
  int x = 0, y = 0, id = 0;
  //printf("making crates\n");
  for(int j = 0; j < rows; j++){
      y+=70;
      for(int i = 0; i < columns; i++){
	x+=80;
	//bottle = (Item*) malloc(sizeof(Item));
	bottle = mkBottle(x, y, gc);
	head = insertItem(head, bottle);
	head = insertItem(head, mkRectangleItem(x+9, y, 6, 4, gc));
	head = insertItem(head, mkRectangleItem(x-9, y, 6, 4, gc));
	bottles[j][i] = (JinneeBottle*)malloc(sizeof(JinneeBottle));
	bottles[j][i]->bottle = bottle;
	//printf("made a bottle\n");
	bottles[j][i]->x = x;
	bottles[j][i]->y = y;
	bottles[j][i]->width_modifier = 20;
	bottles[j][i]->height = 55;
	bottles[j][i]->id = ++id;
	bottles[j][i]->moving = 0;
	bottles[j][i]->closed = 0;
	printf("id: %i", id);
	
      }
      x = 0;
  }
  return head;
}

/*
 *======================================================
 * Function to create a cork for each bottle in the
 * crate
 *======================================================
 */
ListPtr mkCorks(JinneeBottle*** bottles, ListPtr corks, int rows, int columns, GC gc) {
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < columns; j++){
      corks = insertItem(corks, mkCork(bottles, i, j, gc));
    }
  }
  return corks;
}

/*
 *======================================================
 * Function to create a jinnee for each bottle in the
 * crate
 *======================================================
 */
ListPtr mkJinnees(JinneeBottle*** bottles, ListPtr head, int rows, int columns, GC gc){
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < columns; j++){
      head = insertItem(head, mkJinnee(bottles, i, j, gc));
    }
  }
  return head;
}

/*
 *======================================================
 * Function to see if a click was in a specific region
 *======================================================
 */
int inBox(int clickedx, int clickedy, int ox, int oy, int h, int w_m){
  //printf("%i, %i to %i, %i w_m: %i h: %i. y range: %i\n",clickedx, clickedy, ox, oy, w_m, h, (oy <= clickedy && clickedy <= h));
  if( oy <= clickedy && clickedy <= oy+h && ox-w_m <= clickedx && clickedx <= ox+w_m)
    return 1;
  else return 0;
}
  
/*
 *======================================================
 * Function to check if a mouse click hit a bottle
 *======================================================
 */
JinneeBottle* hitBottle(JinneeBottle*** bottles, int x, int y){
  int row;
  int column;
  JinneeBottle *cur;
  for(int i = 0; i<5; i++){
    for(int j = 0; j<5; j++){
      cur = bottles[i][j];
      if(inBox( x, y, cur->x, cur->y, cur->height, cur->width_modifier))
	return cur;
    }
  }
  return 0;
}

/*
 *======================================================
 * Function to check if all of a matirx is true (1)
 *======================================================
 */
int checkMatrix(int** matrix){
  int num = 0;
  for(int i = 0; i<5; i++){
    for(int j = 0; j<5; j++){
      num++;
      if(!matrix[i][j]==1)
	//printf("num: %i\n",num);
	return 0;
    }
  }
  return 1;
}

JinneeBottle* randomJinnee(JinneeBottle*** jbottles, int** nums){
  JinneeBottle* bottle;
  int x = rand() % 5;
  int y = rand() % 5;
  bottle = jbottles[x][y];
  if(!bottle->moving && !bottle->closed && nums[x][y] == 0){
    return bottle;
  }
  else if(checkMatrix(nums))
    return 0;
  else {
    nums[x][y] = 1;
    return randomJinnee(jbottles, nums);
  }
}

int animateList(linkedList* list){
  struct linkListNode* current;
  current = list->first;
  while(current != 0){
    //current->y vs current->cork->y
    printf("%i\n", current->bottle->y - current->bottle->cork->y);
    if(current->bottle->y - current->bottle->cork->y < CorkOffset){
      current->bottle->cork->y--;
      current->bottle->jinnee->y--;
    }
    else{
      printf("FUCK!!!\n");
      return 1;
    }
    current = current->next;
 } 
  return 0;
}

int checkClosed(JinneeBottle*** jbottles){
  int num = 0;
  for(int i = 0; i<5; i++){
    for(int j = 0; j<5; j++){
      num++;
      if(!jbottles[i][j]->closed)// || !jbottles[i][j]->moving)
	//printf("num: %i\n",num);
	return 0;
    }
  }
  return 1;
}

/*
 *======================================================
 * Start executing here.
 *   First initialize window.
 *   Next loop responding to events.
 *   Exit forcing window manager to clean up 
 *======================================================
 */

int main (int argc, char *argv[] ) {
  ListPtr head, corks, next;
  Display *display;
  XSizeHints hints;
  GC gc, gc2, gc3, gc4;
  KeySym key;
  XEvent event;
  Window window;
  int screen;
  int i;
  JinneeBottle* hit;
  double start;
  char text[BufferSize];
  JinneeBottle*** jbottles;
  double sleeptime = SleepTime;
  double printTime;
  int playing;
  double jinneeGap, lastJinnee;
  linkedList* movingJinnees;
  int jinneeOut, jinneeAdded;
  int** matrix;

   matrix = (int**)malloc(sizeof(int*)*5);
   for(int i = 0; i<5; i++)
     matrix[i] = (int*)malloc(sizeof(int)*5);

  srand(getTime());

  jinneeGap = JinneeGap;
  jinneeOut = 0;

  playing = 1;

  
  /* corks = (ListPtr) malloc(sizeof(ListPtr)); */
  /* corks = 0; */


  jbottles = (JinneeBottle***) malloc(sizeof(JinneeBottle**)*5);
  for(int i = 0; i<5; i++){
    jbottles[i] = (JinneeBottle**) malloc(sizeof(JinneeBottle*)*5);
  }
/*
 * Display opening uses the DISPLAY  environment variable.
 * It can go wrong if DISPLAY isn't set, or you don't have permission.
 */  
  display = XOpenDisplay( NULL );
  if ( !display ) {
    error( "Can't open display." );
  }

/*
 * Find out some things about the display you're using.
 */
  screen = DefaultScreen( display );
  background = WhitePixel( display, screen );
  foreground = BlackPixel( display, screen );

/*
 * Set up hints and properties for the window manager, and open a window.
 * Arguments to XCreateSimpleWindow :
 *                 display - the display on which the window is opened
 *                 parent - the parent of the window in the window tree
 *                 x,y - the position of the upper left corner
 *                 width, height - the size of the window
 *                 Border - the width of the window border
 *                 foreground - the colour of the window border
 *                 background - the colour of the window background.
 * Arguments to XSetStandardProperties :
 *                 display - the display on which the window exists
 *                 window - the window whose properties are set
 *                 Hello1 - the title of the window
 *                 Hello2 - the title of the icon
 *                 none - a pixmap for the icon
 *                 argv, argc - a comand to run in the window
 *                 hints - sizes to use for the window.
 */
  hints.x = 100;
  hints.y = 100;
  hints.width = 500;
  hints.height =500;
  hints.flags = PPosition | PSize;
  window = XCreateSimpleWindow( display, DefaultRootWindow( display ),
                                hints.x, hints.y, hints.width, hints.height,
                                Border, foreground, background );
  XSetStandardProperties( display, window, "Hello1", "Hello2", None,
                                argv, argc, &hints );

/*
 * Get a graphics context and set the drawing colours.
 * Arguments to XCreateGC
 *           display - which uses this GC
 *           window - which uses this GC
 *           GCflags - flags which determine which parts of the GC are used
 *           GCdata - a struct that fills in GC data at initialization.
 */
  gc = XCreateGC (display, window, 0, 0 );
  XSetBackground( display, gc, background );
  XSetForeground( display, gc, foreground );
  XSetLineAttributes(display, gc, 3,              // 3 is line width
             LineSolid, CapButt, JoinRound);         // other line options

  gc2 = XCreateGC(display, window, 0, 0);
   XSetBackground( display, gc2, background);
   XSetForeground( display, gc2, foreground);
   XSetLineAttributes(display, gc2, 7, LineSolid, CapButt, JoinRound);

  gc3 = XCreateGC(display, window, 0, 0);
   XSetBackground( display, gc3, background);
   XSetForeground( display, gc3, foreground);
   XSetLineAttributes(display, gc3,70, LineSolid, CapButt, JoinRound);

   gc4 = XCreateGC(display, window, 0, 0);
   XSetBackground( display, gc4, foreground);
   XSetForeground( display, gc4, background);
   XSetLineAttributes(display, gc4, 7, LineSolid, CapButt, JoinRound);

/*
 * Tell the window manager what input you want.
 */
  XSelectInput( display, window,
                ButtonPressMask | KeyPressMask | ExposureMask );

/*
 * Put the window on the screen.
 */
  XMapRaised( display, window );

  /*
   * make splash screen
   */
  head = 0;
  head = insertItem(head, mkStringItem(200, 100, "Welcome to Jinnee!", gc));
  head = insertItem(head, mkBottle(250, 250, gc));
  head = insertItem(head, mkRectangleItem(250+9, 250, 6, 4, gc));
  head = insertItem(head, mkRectangleItem(250-9, 250, 6, 4, gc));
  //head = insertItem(head, mkRectangleItem( 250-4, 250, 13, 12, gc));
  head = insertItem(head, mkEllipseItem(250-2, 250 + 18, 7, 7, 0, 360, gc2));
  head = insertItem(head, mkStringItem(215, 390, "Click to Play", gc));
      
  /*
   * Display Splash Screen until screen is clicked or program is quit
   * by q key
   */
  repaint( head, display, window, gc);
  splash = 1;
  while(splash){
    XNextEvent( display, &event);
    switch( event.type) { 
    case Expose:
      if ( event.xexpose.count == 0 ) {
        repaint( head, display, window, gc );
      }
      break;
    case ButtonPress:
      //gets game set up for when splash screen goes away
      splash = 0;
     
      head = 0;
      head = mkCrate(jbottles, head, 5, 5, gc);
      head = mkJinnees(jbottles, head, 5, 5, gc2);
      head = mkCorks(jbottles, head, 5, 5, gc);
      //paints start of game
      repaint( head, display, window, gc );         
      break;
    case KeyPress:
      i = XLookupString( (XKeyEvent *)&event, text, BufferSize, &key, 0 );
	if ( i == 1 && text[0] == 'q' ) {
	  error( "Terminated normally." );
	}
	break;
    }
  }
  
  //creates list of currently animated objects and adds a random
  //bottle to it
  movingJinnees = createLL(randomJinnee(jbottles, matrix));
  movingJinnees->first->bottle->moving = 1;
  //printf("Jinnee added to animations\n");
  lastJinnee = getTime();
  
  //printf("time %f\n", lastJinnee);
  //loop that updates and executes the game
  while(playing) {
    start = getTime();
    //printf("%f\n", start);
    if ( XEventsQueued( display, 0 ) != 0 ){
      XNextEvent( display, &event );
      switch( event.type ) {
	/*
	 * Repaint the window on expose events.
	 */
      case Expose:
      	if ( event.xexpose.count == 0 ) {
      	  repaint( head, display, window, gc );
      	}
      	break;
	/*
	 * Closes bottles that are opening on click
	 * Ends game on bottle smash (when click on bottle
	 * that is not opening)
	 */
      case ButtonPress:
	hit = hitBottle(jbottles, event.xbutton.x, event.xbutton.y);
	if(hit != 0){
	  printf("Bottle number %i was clicked.\n", hit->id);
	  if(hit->moving){
	    hit->moving = 0;
	    hit->closed = 1;
	    hit->cork->y = hit->y;
	    hit->jinnee->y = hit->y+24;
	    hit->jinnee->width--;
	    hit->jinnee->height--;
	    removeItemLL(movingJinnees, hit);

	    if(movingJinnees->first == 0 && checkClosed(jbottles)){
	      playing = 0;
	    }
	  }
	  else if(!hit->closed){
	    jinneeOut = 1;
	    playing = 0;
	  }
	}
	break;
	/*
	 * Exit when 'q' is typed.
	 * Decrease frame rate on '>'
	 * Increase frame rate on '<'
	 * Increase time between jinn trying to escape on '.'
	 * Decrease time between jinn trying to escape on ','
	 * Arguments for XLookupString :
	 *                 event - the keyboard event
	 *                 text - buffer into which text will be written
	 *                 BufferSize - size of text buffer
	 *                 key - workstation independent key symbol
	 *                 0 - pointer to a composeStatus structure
	 */
      case KeyPress:
	i = XLookupString( (XKeyEvent *)&event, text, BufferSize, &key, 0 );
	if ( i == 1 && text[0] == 'q' ) 
	  error( "Terminated normally." );
	else if(i == 1 && text[0] == '>')
	  sleeptime +=5000;
	else if(i == 1 && text[0] == '<')
	  sleeptime -= 5000;
	else if(i == 1 && text[0] == '.')
	  jinneeGap +=500000;
	else if(i == 1 && text[0] == ',') 
	  jinneeGap -=500000;
	break;
      }
    }
    //end of if
    //printf("next jinnee %f\n",getTime()-lastJinnee);
    /*
     * Opens a new bottle if it has been enough time since
     * the last opened. if all have been closed, game is won
     */
    if(getTime()-lastJinnee > jinneeGap){
      if(checkClosed(jbottles)){
	playing = 0;
	//break;
      }
      else{
	jinneeAdded = addItemLL(movingJinnees, randomJinnee(jbottles, matrix));
	//printf("Jinnee added to animations List\n");
	if(jinneeAdded){
	  movingJinnees->first->bottle->moving = 1;
	  lastJinnee = getTime();
	}
	else{ 
	  if(checkClosed(jbottles)){
	    playing = 0;
	  }
	}
      }
    }
    /*
     * animates corks and jinn that are currently in motion
     */
    if(!jinneeOut)
      jinneeOut = animateList(movingJinnees);
    printf("Animated!!!!\n");
    if(jinneeOut == 1)
      playing = 0;
    //paints current game state
    repaint( head, display, window, gc );
    printTime = start + sleeptime - getTime();
    printf("time %f\n", printTime);
    //sleeps process to have uniform frame/refresh rate
    usleep(printTime);
  }

 
  head = 0;
  printf("out: %i\n",jinneeOut);
  /*
   * Sets up Losing Screen
   */
  if(jinneeOut == 1){
    head = insertItem(head, mkStringItem(230, 100, "You Lost!", gc));
    
    //left jinnee
    head = insertItem(head, mkEllipseItem(120-2, 280, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(120-21, 290, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(100-2, 280 + 18, 20, 20, 0, 360, gc3));

    //middle jinnee
    head = insertItem(head, mkEllipseItem(215-2, 180, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(215-21, 180, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(200-2, 160 + 18, 20, 20, 0, 360, gc3));

    //right jinnee
    head = insertItem(head, mkEllipseItem(360-2, 234, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(360-21, 240, 10, 10, 0, 360, gc4));
    head = insertItem(head, mkEllipseItem(360-2, 220 + 18, 20, 20, 0, 360, gc3));

    head = insertItem(head, mkStringItem(180, 300, "The Jinn have Excaped!!", gc));
  }
  /*
   * Sets up Winning Screen
   */
  else{
    head = insertItem(head, mkStringItem(230, 100, "You Prevented a Disaster!\n The jinn are still in their bottles.", gc));
  }      
  
  /*
   * Paints Last Screen
   */  
  repaint( head, display, window, gc);

  /*
   * Checks if the user has tried to quit yet
   */
  while(end){
    XNextEvent( display, &event);
    switch( event.type) { 
    case Expose:
      if ( event.xexpose.count == 0 ) {
        repaint( head, display, window, gc );
      }
      break;
    case KeyPress:
	i = XLookupString( (XKeyEvent *)&event, text, BufferSize, &key, 0 );
	if ( i == 1 && text[0] == 'q' ) {
	  error( "Terminated normally." );
	}
	break;
    }
  }
}

