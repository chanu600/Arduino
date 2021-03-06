#include <LiquidCrystal.h>
#include "MenuEntry.h"
#include "MenuLCD.h"
#include "MenuManager.h"

//This example is a Stopwatch and Timer.  Although it is mostly functional, it might not be the best
// user interface.  The layout was created more to provide examples of a stopwatch/timer.


//programado para el display dfrobot
const int LCDD7 = 7;
const int LCDD6 = 6;
const int LCDD5 = 5;
const int LCDD4 = 4;
const int LCDE  = 9;
const int LCDRS = 8;

const int BOTONES = 0;

int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
long timerbotones = 0 ;
#define tiempobotones 150


//Now create the MenuLCD and MenuManager classes.
//#define DISPLAYCREADOACA 1 //esto es si crea el objeto display el menu o si ya viene creado
#ifdef DISPLAYCREADOACA //si no esta definido en otro lado el liquidcrystal:
  MenuLCD g_menuLCD( LCDRS, LCDE, LCDD4, LCDD5, LCDD6, LCDD7, 16, 2);
#else //si esta definido de antes:
  MenuLCD g_menuLCD (16, 2);
  LiquidCrystal pepe (LCDRS, LCDE, LCDD4, LCDD5, LCDD6, LCDD7);
#endif
// y poner en el setup setLCD;



MenuManager g_menuManager( &g_menuLCD);//pass the g_menuLCD object to the g_menuManager with the & operator.

//Global variables used by the sample application
//when the display is showing user results (e.g. time elapsed), the next "select" should send it back into the menu.
unsigned int g_isDisplaying = false;
int g_timerTime = 23;
long g_timerRunning = false;
long g_timerTarget = 0;
long g_autoReset = false;
long g_stopMillis = 0;
long g_startMillis = 0;

byte g_smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

byte g_frown[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B00000,
  B01110,
  B10001,
};


//end Global variables section

//setupMenus
//This function is called during setup to populate the menu with the tree of nodes
//This can be a bit brain-bending to write.  If you draw a tree you want for your menu first
// this code can be a little easier to write.  Add the nodes in a depth-first order for
// the easiest code and least amount of temporary variables.
// http://en.wikipedia.org/wiki/Depth-first
// The MenuManager code is the same for building the menu as for selecting it via inputs.
// You create the menu entries and then move about the menu structure using Up, Down, Select as if you
// were selecting them via the inputs, and then use either AddChild or Add sibling from the selected node
// to create your menu.
//
//  This sample code is a simple stopwatch.  Our menu will look like this:
//  Stopwatch
//  |-Start
//  |-Stop
//  |-Reset
//  Timer
//  |-Set Time
//  |-AutoReset
//  | |-On
//  | |-Off
//  |-Start
//  |-Stop
//  Credits


void setupMenus()
{  
  
  //Add nodes via a depth-first traversal order
  MenuEntry * p_menuEntryRoot;
  //Add root node
  //MenuEntry( char * menuText, void * userData/*=0*/, MENU_ACTION_CALLBACK_FUNC func);
  p_menuEntryRoot = new MenuEntry("Stopwatch", NULL, NULL);
  g_menuManager.addMenuRoot( p_menuEntryRoot );
  g_menuManager.addChild( new MenuEntry("Stopwatch Start", NULL, WatchStartCallback) );
  g_menuManager.addChild( new MenuEntry("Stopwatch Stop", NULL, WatchStopCallback ) );
  g_menuManager.addChild( new MenuEntry("Reset", NULL, WatchResetCallback) );
  g_menuManager.addChild( new MenuEntry("Back", (void *) &g_menuManager, MenuEntry_BackCallbackFunc) );
  
  g_menuManager.addSibling( new MenuEntry("Timer", NULL, NULL ) );
  //Now we want to select the "Timer" entry so we can add children under that node
  //"Timer" is one down from "Stopwatch", so we issue the down command
  g_menuManager.MenuDown();
  g_menuManager.addChild( new MenuEntry("Set Time", NULL, SetTimeCallback ) );
  //now move down to the "Time" node to add children by selecting the "Timer" node 
  g_menuManager.MenuSelect(); 
  //Add "time"'s sibling "AutoReset" and select it
  g_menuManager.addSibling( new MenuEntry( "AutoReset", NULL, NULL) );
  g_menuManager.MenuDown();

  //Add "AutoReset"'s children
  //Use the built-in BOOL setting callbacks from MenuEntry.h: MenuEntry_Bool*CallbackFunc
  g_menuManager.addChild( new MenuEntry( "Turn Reset On",  (void *) (&g_autoReset), MenuEntry_BoolTrueCallbackFunc ) );
  g_menuManager.addChild( new MenuEntry( "Turn Reset Off", (void *) (&g_autoReset), MenuEntry_BoolFalseCallbackFunc ) );
  g_menuManager.addChild( new MenuEntry("Back", (void *) &g_menuManager, MenuEntry_BackCallbackFunc) );

  //Add timer start and stop
  g_menuManager.addSibling( new MenuEntry( "Countdown Start", NULL, TimerStartCallback) );
  g_menuManager.addSibling( new MenuEntry( "Countdown Stop", NULL, TimerStopCallback) );
  g_menuManager.addSibling( new MenuEntry("Back", (void *) &g_menuManager, MenuEntry_BackCallbackFunc) );
  
  //Get the selection state back to the root for startup and to add the last entry
  g_menuManager.SelectRoot();
  g_menuManager.addSibling( new MenuEntry( "Credits", NULL, CreditsCallback) );
  //Make sure the menu is drawn correctly after all changes are done
  g_menuManager.DrawMenu();

  g_menuManager.addSibling( new MenuEntry( "Draw Smiley", NULL, SmileyCallback) );
  
  g_menuLCD.getLCD()->createChar( 0, g_smiley );
  g_menuLCD.getLCD()->createChar( 1, g_frown );
}



void setup() 
{           
  Serial.begin(115200);
  Serial.print("Ready.");
  
  #ifdef DISPLAYCREADOACA
    g_menuLCD.MenuLCDSetup();  
  #else
    g_menuLCD.setLCD(&pepe);
  pepe.begin(16,2);
  #endif
  
  setupMenus();
}

void loop() 
{
  //The example shows using bytes on the serial port to move the menu. You can hook up your buttons or other controls.
  char incomingByte = 0;
  if (Serial.available() > 0) 
  {
    incomingByte = Serial.read();
  }
  switch( incomingByte )
  {
    case 'u':
      g_menuManager.DoMenuAction( MENU_ACTION_UP );
      break;
    case 'd':
      g_menuManager.DoMenuAction( MENU_ACTION_DOWN );
      break;
    case 's':
      if( g_isDisplaying )
      {
        g_isDisplaying = false;
        g_menuManager.DrawMenu();
      }
      else
      {
      g_menuManager.DoMenuAction( MENU_ACTION_SELECT );
      }
      break;
    case 'b':
      g_menuManager.DoMenuAction( MENU_ACTION_BACK );
      break;
    default:
      break;
  }
  switch( read_LCD_buttons() )
  {
    case btnUP:
      g_menuManager.DoMenuAction( MENU_ACTION_UP );
      break;
    case btnDOWN:
      g_menuManager.DoMenuAction( MENU_ACTION_DOWN );
      break;
    case btnSELECT:
      if( g_isDisplaying )
      {
        g_isDisplaying = false;
        g_menuManager.DrawMenu();
      }
      else
      {
      g_menuManager.DoMenuAction( MENU_ACTION_SELECT );
      }
      break;
    case btnLEFT:
      g_menuManager.DoMenuAction( MENU_ACTION_BACK );
      break;
    default:
      break;
  }
  
  if( g_timerRunning && g_timerTarget < millis())
  {
    long time = millis();
    
    Serial.print( "time = " );
    Serial.println( time );
    
    Serial.println("Timer Goes Off HERE!");
    if( g_autoReset)
    {
      Serial.print( "timer target was "); 
      Serial.println( g_timerTarget );
      g_timerTarget = time + ( g_timerTime *1000 );
      Serial.print( "timer target is now ");
      Serial.println( g_timerTarget );
      Serial.print( "timerTime is " );
      Serial.println( g_timerTime );
      Serial.println( "--------" );
    }
    else
    {
      g_timerRunning = false;
    }
  }
}

int read_LCD_buttons()
{
  if (millis()-timerbotones > tiempobotones) 
  {
     timerbotones = millis();
     adc_key_in = analogRead(BOTONES);      // read the value from the sensor 
     // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
     // we add approx 50 to those values and check to see if we are close
     if (adc_key_in > 1000) 
     {
      return btnNONE;
     }
     // We make this the 1st option for speed reasons since it will be the most likely result
     // For V1.1 us this threshold
     if (adc_key_in < 50)  
     {
     return btnRIGHT;  
     }
     
     if (adc_key_in < 99)  
     {
      return btnUP; 
     }
     if (adc_key_in < 256)  
     {
      return btnDOWN;
     } 
     if (adc_key_in < 408)  
     {
        return btnLEFT; 
     }
     
     if (adc_key_in < 638)  
     {
        return btnSELECT;  
     }
     
  
     
     return btnNONE;  // when all others fail, return this...
  }
}

//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchStartCallback( char* pMenuText, void *pUserData )
{
  g_startMillis = millis();
  char *pTextLines[2] = {"Clock Started", "" };
  g_menuLCD.PrintMenu( pTextLines, 2, 3 );
  g_isDisplaying = true;  
}


//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchStopCallback( char* pMenuText, void *pUserData )
{
  g_stopMillis = millis();
  
  char strSeconds[50];
  dtostrf( ((float)(g_stopMillis-g_startMillis))/1000, 1, 2, strSeconds );
  char *pTextLines[2] = {"Elapsed Time", strSeconds };
  g_menuLCD.PrintMenu( pTextLines, 2, 3 );
  g_isDisplaying = true;
}  
  
//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchResetCallback( char* pMenuText, void *pUserData )
{
  g_startMillis = 0;
  g_stopMillis = 0;
  char *pTextLines[2] = {"Clock reset", "" };
  g_menuLCD.PrintMenu( pTextLines, 2, 3 );
}

//This callback uses the built-in Int Input routine in MenuManager.h to request input of a integer number from the 
//user.  Control will pass to the DoIntInput function until the user finishes.  the g_timerTime will be set to the 
//value the user selects.
void SetTimeCallback( char* pMenuText, void *pUserData )
{
  char *pLabel = "Timer seconds";
  int iNumLabelLines = 1;
  int iMin = 1;
  int iMax = 1000;
  int iStart = 60;
  //Each user input action (such as a turn of rotary enocoder or push of button
  //will step this amount
  int iStep = 5;
  
  g_menuManager.DoIntInput( iMin, iMax, iStart, iStep, &pLabel, iNumLabelLines, &g_timerTime );
  Serial.print("Timer time" );
  Serial.println( g_timerTime );
}
//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void TimerStartCallback( char* pMenuText, void *pUserData )
{
  g_timerTarget = millis() + (g_timerTime * 1000);//This is buggy- doesn't handle wrap-around of the millis output.  Too bad :(  
  Serial.print( "timer target = ");
  Serial.println( g_timerTarget );
  Serial.print( "time = " );
  Serial.println( millis());
  g_timerRunning = true;
  char strSeconds[50];
  itoa( g_timerTime, strSeconds, 10 );
  char *pTextLines[2] = {"Go!", strSeconds };
  g_menuLCD.PrintMenu( pTextLines, 2, 3 );
  g_isDisplaying = true;
}  


//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void TimerStopCallback( char* pMenuText, void *pUserData )
{
  g_timerRunning = false;
}  

void CreditsCallback( char* pMenuText, void *pUserData )
{
  char *pTextLines[2] = {"David Andrews ", "Dustin Andrews" };
  g_menuLCD.PrintMenu( pTextLines, 2, 1 );
  delay(5000);
  char *pTextLines2[2] = {"http://authenti","cinvention.com"};
  g_menuLCD.PrintMenu( pTextLines2, 2, 5 );  
  g_isDisplaying = true;
}  

void SmileyCallback( char* pMenuText, void *pUserData )
{
  for( int i = 0; i < 10 ; ++i )
  {
    g_menuLCD.ClearLCD();
    g_menuLCD.getLCD()->setCursor( 8,0 );
    g_menuLCD.getLCD()->print( (char)0 );
    delay(500);
    g_menuLCD.ClearLCD();
    g_menuLCD.getLCD()->setCursor( 8,0 );
    g_menuLCD.getLCD()->print( (char)1 );
    delay(500);
  }   
}
