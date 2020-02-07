// running on an atmega328

#include "av.h"
#include "musictrack.h"

static const uint8_t PROGMEM waveSine[] = 
{
    127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
    176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
    217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
    245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
    255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
    245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
    217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
    176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
    127,124,121,118,115,111,108,105,102, 99, 96, 93, 90, 87, 84, 81,
     78, 75, 72, 70, 67, 64, 61, 59, 56, 54, 51, 49, 46, 44, 41, 39,
     37, 35, 33, 30, 28, 26, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
      9,  8,  7,  6,  5,  4,  3,  3,  2,  1,  1,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,  8,
      9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 26, 28, 30, 33, 35,
     37, 39, 41, 44, 46, 49, 51, 54, 56, 59, 61, 64, 67, 70, 72, 75,
     78, 81, 84, 87, 90, 93, 96, 99,102,105,108,111,115,118,121,124
};
static const uint8_t PROGMEM waveTriangle[] = 
{
      0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 
     32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
     64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94,
     96, 98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
    128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
    160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
    192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
    224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
    255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,
    223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,
    191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,
    159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,
    127,125,123,121,119,117,115,113,111,109,107,105,103,101, 99, 97,
     95, 93, 91, 89, 87, 85, 83, 81, 79, 77, 75, 73, 71, 69, 67, 65,
     63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33,
     31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11,  9,  7,  5,  3,  1
};

static const uint8_t PROGMEM screenTiles[] = 
{
#include "tetris_tiles.h"
};

static const uint8_t PROGMEM pieceShapes[] = 
{
    // 0 = The J piece
    0, 0, TILE_J, 0,
    0, 0, TILE_J, 0,
    0, TILE_J, TILE_J, 0, 
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_J, 0, 0,
    0, TILE_J, TILE_J, TILE_J,
    0, 0, 0, 0, 
    //
    0, 0, 0, 0,
    0, TILE_J, TILE_J, 0,
    0, TILE_J, 0, 0,
    0, TILE_J, 0, 0, 
    //
    0, 0, 0, 0,
    TILE_J, TILE_J, TILE_J, 0,
    0, 0, TILE_J, 0, 
    0, 0, 0, 0,
    //
    // 1 = The L piece
    0, TILE_L, 0, 0,
    0, TILE_L, 0, 0,
    0, TILE_L, TILE_L, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_L, TILE_L, TILE_L,
    0, TILE_L, 0, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_L, TILE_L, 0,
    0, 0, TILE_L, 0,
    0, 0, TILE_L, 0,
    //
    0, 0, 0, 0,
    0, 0, TILE_L, 0,
    TILE_L, TILE_L, TILE_L, 0,
    0, 0, 0, 0,
    //
    // 2 = The O piece
    0, 0, 0, 0,
    0, TILE_O, TILE_O, 0,
    0, TILE_O, TILE_O, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_O, TILE_O, 0,
    0, TILE_O, TILE_O, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_O, TILE_O, 0,
    0, TILE_O, TILE_O, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_O, TILE_O, 0,
    0, TILE_O, TILE_O, 0,
    0, 0, 0, 0,
    //
    // 3 = The S piece
    0, 0, 0, 0,
    0, TILE_S, TILE_S, 0,
    TILE_S, TILE_S, 0, 0,
    0, 0, 0, 0,
    //
    0, TILE_S, 0, 0,
    0, TILE_S, TILE_S, 0,
    0, 0, TILE_S, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    0, TILE_S, TILE_S, 0,
    TILE_S, TILE_S, 0, 0,
    0, 0, 0, 0,
    //
    0, TILE_S, 0, 0,
    0, TILE_S, TILE_S, 0,
    0, 0, TILE_S, 0,
    0, 0, 0, 0,
    //
    // 4 = The T piece
    0, 0, 0, 0,
    TILE_T, TILE_T, TILE_T, 0,
    0, TILE_T, 0, 0,
    0, 0, 0, 0,
    //
    0,  TILE_T, 0, 0,
    TILE_T, TILE_T, 0, 0,
    0, TILE_T, 0, 0,
    0, 0, 0, 0,
    //
    0, TILE_T, 0, 0,
    TILE_T, TILE_T, TILE_T, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    //
    0, TILE_T, 0, 0,
    0, TILE_T, TILE_T, 0,
    0, TILE_T, 0, 0,
    0, 0, 0, 0,
    //
    // 5 = The Z piece
    0, 0, 0, 0,
    TILE_Z, TILE_Z, 0, 0,
    0, TILE_Z, TILE_Z, 0,
    0, 0, 0, 0,
    //
    0, 0, TILE_Z, 0,
    0, TILE_Z, TILE_Z, 0, 
    0, TILE_Z, 0, 0,
    0, 0, 0, 0,
    //
    0, 0, 0, 0,
    TILE_Z, TILE_Z, 0, 0,
    0, TILE_Z, TILE_Z, 0,
    0, 0, 0, 0,
    //
    0, 0, TILE_Z, 0,
    0, TILE_Z, TILE_Z, 0, 
    0, TILE_Z, 0, 0,
    0, 0, 0, 0,
    //
    // 6 = The I piece
    0, 0, 0, 0,
    0, 0, 0, 0,
    TILE_H0, TILE_H1, TILE_H2, TILE_H3,
    0, 0, 0, 0,
    //
    0, TILE_I0, 0, 0,
    0, TILE_I1, 0, 0,
    0, TILE_I2, 0, 0,
    0, TILE_I3, 0, 0,
    // 
    0, 0, 0, 0,
    0, 0, 0, 0,
    TILE_H0, TILE_H1, TILE_H2, TILE_H3,
    0, 0, 0, 0,
    //
    0, TILE_I0, 0, 0,
    0, TILE_I1, 0, 0,
    0, TILE_I2, 0, 0,
    0, TILE_I3, 0, 0,
};


#define PFWIDTH 10
#define PFHEIGHT 20
uint8_t playfield[PFWIDTH][PFHEIGHT];
byte piecex;
byte piecey;
byte piecetype;
byte piecerotation;
byte nextpieces[7];
byte nextcursor;
byte turntimeleft;
bool speedup;
int lines;
long score;
bool muted = false;

#define BUTTON_LEFT   0x01
#define BUTTON_RIGHT  0x02
#define BUTTON_UP     0x04
#define BUTTON_DOWN   0x08
#define BUTTON_A      0x10
#define BUTTON_B      0x20
#define BUTTON_START  0x40
#define BUTTON_SELECT 0x80

byte levelspeed[21] = { 40, 30, 27, 24, 22, 20, 18, 16, 14, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 }; 

void setup()
{
    DDRC = 0;     // use port c for inputs
    PORTC = 0x3f; // weak pull-up
    DDRB = 0;     // use port b for inputs
    PORTB = 0x18; // weak pull-ups 
      
    av_init(screenTiles, waveSine); 
}
void loop() 
{  
    Musictrack part1[4] = {
        //                                 |               |               |               |               |               |               |               |               |
        Musictrack(voiceA, 90, waveTriangle, 5, F("q---l-m-o-qom-l-j---j-m-q---o-m-l---llm-o---q---m---j---j-------o---o-r-v---t-r-q---q-m-q---o-m-l---llm-o---q---m---j---j------ ")),
        Musictrack(voiceB, 0,  waveTriangle, 5, F("                                                                                                                                ")),
        Musictrack(voiceC, 80, waveSine,     5, F("Q-- Q-- Q-- Q-- V-- V-- V-- V-- U-- U-- Q-- Q-- V-- V-- V-- a-- O-- O-- O-- O-- M-- M-- M-- M-- H-- H-- Q-- Q-- V-- V-- V------ ")),
        Musictrack(voiceD, 70, waveSine,     5, F("  e-- e-- e-- e-- j-- j-- j-- j-- j-- j-- e-- e-- j-- j-- X-- c-- c-- c-- c-- c-- a-- a-- a-- a-- T-- T-- e-- e-- j-- j-- j--   "))
    };
    Musictrack part2[4] = {
        //                                 |               |               |               |               |               |               |               |               | 
        Musictrack(voiceA, 70, waveTriangle, 5, F("m-------j-------l-------i-------j-------e-------e-------i-------m-------j-------l-------i-------j---m---q---q---q-------------- ")),
        Musictrack(voiceB, 70, waveTriangle, 5, F("q-------m-------o-------l-------m-------j-------i-------l-------q-------m-------o-------l-------m---q---v---v---u-------------- ")),
        Musictrack(voiceC, 60, waveSine,     5, F("V-- V-- V-- V-- X-- X-- X-- X-- V-- V-- V-- V-- U-- U-- U-- U-- V-- V-- V-- V-- X-- X-- X-- X-- V-- V-- V-- V-- U-- U-- U-- U-- ")),
        Musictrack(voiceD, 60, waveSine,     5, F("  e-- e-- e-- e-- c-- c-- c-- c-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- c-- c-- c-- c-- e-- e-- e-- e-- e-- e--       "))
    };
    Song song(4, part1, part1, part2);

    // simple start screen
    paintBackground();
    paintBox(4,5, 6,5, false);
    paintText(4,6, "TET\173IS");
    paintText(4,8, "press");
    paintText(4,9, "START");
    while ( (nextFrame(NULL) & BUTTON_START) == 0) { random(100); }

    muted = false;

    for (;;)
    {
      // start the music 
      song.start();
      if (muted) song.silence();
      
      int i,j;
      
      // start one round
      for (i=0; i<PFHEIGHT; i++) for (j=0; j<PFWIDTH; j++) playfield[j][i] = 0;
      generateNextPieces();
      piecex = 4;
      piecey = 3;
      piecerotation = 0;
      nextcursor = 0;
      piecetype = nextpieces[nextcursor++];
      turntimeleft = levelspeed[computeLevel()];
      speedup = false;
      score = 0;
      lines = 0;
      
      paintBackground();
      paintPlayfield();
      paintNextPiece();
      paintStats();
      
      // game loop for one round
      for (;;) 
      {
          // wait for next video frame
          byte clicked = nextFrame(muted ? NULL : &song);
          
          // process user input
          // mute/unmute
          if ((clicked & BUTTON_SELECT) != 0)
          {   muted = !muted;
              if (muted) song.silence();
          }
          // enter pause mode
          if ( (clicked & BUTTON_START) != 0)
          {
              paintBox(4,7, 6,1, false);
              paintText(4,7,"PAUSE");
              song.silence();
              while ( (nextFrame(NULL) & BUTTON_START) == 0) { random(100); }
              paintPlayfield();          
              setPiece(piecex, piecey, piecetype, piecerotation, false, true);
              continue;                      
          }
  
          // controlling the piece
          if ((clicked & BUTTON_LEFT) && piecex>0) 
          {   if (!setPiece(piecex-1,piecey,piecetype,piecerotation,false,false))
              {   piecex--;
              }
          }
          if ((clicked & BUTTON_RIGHT) && piecex<100)
          {   if (!setPiece(piecex+1,piecey,piecetype,piecerotation,false,false))
              {   piecex++;
              }
          }
          if (clicked & BUTTON_A)
          {
              if (!setPiece(piecex,piecey,piecetype,(piecerotation+1)&3,false,false))
              {   piecerotation = (piecerotation+1) & 3;
              } 
              else if (!setPiece(piecex-1,piecey,piecetype,(piecerotation+1)&3,false,false))
              {   piecerotation = (piecerotation+1) & 3;
                  piecex--;
              } 
              else if (!setPiece(piecex+1,piecey,piecetype,(piecerotation+1)&3,false,false))
              {   piecerotation = (piecerotation+1) & 3;
                  piecex++;
              } 
          }
          if (clicked & BUTTON_B)
          {
              if (!setPiece(piecex,piecey,piecetype,(piecerotation-1)&3,false,false))
              {   piecerotation = (piecerotation-1) & 3;
              }
              else if (!setPiece(piecex-1,piecey,piecetype,(piecerotation-1)&3,false,false))
              {   piecerotation = (piecerotation-1) & 3;
                  piecex--;
              }
              else if (!setPiece(piecex+1,piecey,piecetype,(piecerotation-1)&3,false,false))
              {   piecerotation = (piecerotation-1) & 3;
                  piecex++;
              }
          }
          if (clicked & BUTTON_DOWN)
          {
              speedup = true;
              turntimeleft=0;
          }
  
          // game time progressing
          if (turntimeleft>0)
          {
              turntimeleft--;
          }
          else
          {
              turntimeleft = speedup ? 1 : levelspeed[computeLevel()];
              if (speedup) { score++; }
  
              // piece moves down 
              if (!setPiece(piecex,piecey+1,piecetype,piecerotation,false,false))
              {   piecey++;
              }
              // piece can not move down 
              else
              {
                  // piece locks in place
                  setPiece(piecex,piecey,piecetype,piecerotation,true,false);
  
                  // test for removal of full lines
                  int removed = removeCompleteRows(true, true);
                  if (removed>0)
                  {   byte blinking;
                      byte blinkduration = 40;
                      // peform blinking and then really remove lines
                      for (blinking=0; blinking<blinkduration; blinking++)
                      {   nextFrame(muted ? NULL : &song);
                          paintPlayfield();
                          removeCompleteRows(blinking!=blinkduration-1, ((blinking%10) < 5) && blinking!=blinkduration-1);
                      }
                      // increase score
                      int multi = computeLevel()+1;
                      switch (removed) 
                      {   case 1:   score += 40 * multi; break;
                          case 2:   score += 100 * multi; break; 
                          case 3:   score += 300 * multi; break;
                          case 4:   score += 1200 * multi; break;
                      }
                      // track number of removed lines
                      lines += removed;
                  }
  
                  piecex=4;
                  piecey=3;
                  piecetype = nextpieces[nextcursor++];
                  piecerotation = 0;
                  speedup = false;
   
                  if (nextcursor>6) 
                  {   generateNextPieces();
                      nextcursor=0;
                  } 
  
                  paintNextPiece();
                  paintStats();
   
                  // if the next piece can not be spawned, the game is over
                  if (setPiece(piecex,piecey,piecetype,piecerotation,false,false))
                  {
                      break;
                  }
              }
          }
          
          paintPlayfield();
          setPiece(piecex, piecey, piecetype, piecerotation, false, true);
      }
    
      // after the end of the game
      song.silence();

      paintBox(4,5, 6,5, false);
      paintText(4,5, " GAME");
      paintText(4,6, " OVER");
      paintText(4,7, "     ");
      paintText(4,8, "press");
      paintText(4,9, "START");
      while ( (nextFrame(NULL) & BUTTON_START) == 0) { random(100); }
    }
}

byte prevButtons = 0xff;
byte nextFrame(Song* song)
{
    av_waitForBlanking(); 
      
    if (song) { song->tick(); }

    byte b = getButtons();
    byte clicked = (~b) & prevButtons;
    prevButtons = b;
    return clicked;
}

byte getButtons()
{
    return (PINC&0x3f) | ((PINB&0x18) << 3);
}

byte removeCompleteRows(bool onlyTest, bool blink)
{
    byte i,j;
    byte removedrows=0;
    
    for (i=PFHEIGHT; i!=255; i--)
    {
        byte completerow = true;
        for (j=0; j<PFWIDTH; j++)
        {   
            if (playfield[j][i]==0) { completerow=false; }
        }

        if (completerow)
        {   removedrows++;
            if (blink && i>=4)
            {   for (j=0; j<PFWIDTH; j++) 
                {   paintTile(2+j, i-4, TILE_BLACK);
                }           
            }
        }
        else 
        {   if (!onlyTest)
            {   for (j=0; j<PFWIDTH; j++) 
                {   playfield[j][i+removedrows] = playfield[j][i];
                }
            }
        }
    }
    if (!onlyTest)
    {   for (i=0; i<removedrows; i++)
        {
            for (j=0; j<PFWIDTH; j++) { playfield[j][i] = 0; }
        }
    }
    return removedrows;
}


void paintBackground()
{
    int i,j;
    for (i=0; i<320; i++) { videoMatrix[i] = TILE_BLACK; }
    for (i=0; i<16; i++) 
    {   
        videoMatrix[i*20+1] = TILE_WALL;
        for (j=2; j<12; j++) videoMatrix[i*20+j] = TILE_WHITE;
        videoMatrix[i*20+12] = TILE_WALL;       
    }

    paintBox(14,1, 5,1, false);
    paintText(14,1, "SCORE");
    paintText(13,3, "       ");
    paintBox(14,5, 5,2, false);
    paintText(14,5, "LEVEL");    
    paintBox(14,8, 5,2, true);   
    paintText(14,8, "LINES");    
    paintBox(15,11, 4,4, true);   
    paintTile(14,10, TILE_BOX_LSLIT_EXTRA); 
}

void paintText(byte x, byte y, char* txt)
{
    while (*txt)
    {
        paintTile(x,y,*txt);
        txt++;
        x++;
    }
}

void paintNumber(byte x, byte y, byte width, int n)
{
    byte i;
    for (i=0; i<width; i++)
    {   byte digit = n % 10;
        paintTile(x+width-1-i, y, (n>0 || i==0) ? 48+digit : TILE_WHITE);
        n = n / 10;
    }
}

void paintStats()
{
    if (score>999999) { score = 999999; }
    if (score<10000) { paintNumber(13,3,6, (int) score ); }
    else 
    {   paintNumber(15,3,4, (int) (1000+score%1000) );
        paintNumber(13,3,3, (int) (score/1000) );
    }
    paintNumber(14,6,4, computeLevel());
    paintNumber(14,9,4, lines);
}

inline void paintTile(byte x, byte y, byte t)
{
    videoMatrix[x+y*20] = t;
}

void paintBox(byte x, byte y, byte w, byte h, bool topoverlaps)
{
    byte i,j;
  
    paintTile(x-1,y-1, topoverlaps ? TILE_BOX_LSLIT : TILE_BOX_LT);
    paintTile(x+w,y-1, topoverlaps ? TILE_BOX_RSLIT : TILE_BOX_RT);
    paintTile(x-1,y+h, TILE_BOX_LB);
    paintTile(x+w,y+h, TILE_BOX_RB);
    for (i=0; i<w; i++) {
        paintTile(x+i,y-1, topoverlaps ? TILE_BOX_SLIT : TILE_BOX_T);
        paintTile(x+i,y+h, TILE_BOX_B);
    }
    for (i=0; i<h; i++) 
    {   paintTile(x-1,y+i, TILE_BOX_L);
        paintTile(x+w,y+i, TILE_BOX_R);
        for (j=0; j<w; j++) paintTile(x+j,y+i, TILE_WHITE);
    }        
}

void paintNextPiece()
{
    byte x;
    byte y;
    byte type = nextpieces[nextcursor];
    for (y=0; y<4; y++)
    {   for (x=0; x<4; x++)
        {
            byte tile = pgm_read_byte(pieceShapes+(((int)type)*64+x+y*4));
            paintTile(x+15,y+11,tile); 
        }
    }
}

void paintPlayfield()
{
    byte x=0; 
    byte y=0;
    for (x=0; x<PFWIDTH; x++)
    {
        for (y=0; y<16; y++)
        {
            videoMatrix[20*y+x+2] = playfield[x][y+4];
        }      
    }
}

void clearPlayfield()
{
    byte x,y;
    for (x=0; x<PFWIDTH; x++)
    {
      for (y=0; y<PFHEIGHT; y++)
      {
          playfield[x][y] = 0;
      }
    }
}

bool setPiece(byte x, byte y, byte typ, byte rot, bool toPlayfield, bool toScreen)
{
    bool anycollisions = false;
    byte i;

    for (i=0; i<16; i++)
    { 
        byte tile = pgm_read_byte(pieceShapes+(((int)typ)*64+rot*16+i));
        if (tile!=0)
        {
            byte tx = x - 1 + i%4;
            byte ty = y - 1 + i/4;
            
            if (tx>=PFWIDTH || ty>=PFHEIGHT) { anycollisions=true; }
            else
            {   if (playfield[tx][ty]!=0) { anycollisions=true; }    
                if (toPlayfield) { playfield[tx][ty] = tile; }
                if (toScreen && ty>=4) { videoMatrix[20*(ty-4)+tx+2] = tile; }
            }
        }   
    }
    return anycollisions;
}

void generateNextPieces()
{
    byte i;
    for (i=0; i<7; i++) { nextpieces[i] = i; }
    for (i=0; i<6; i++)
    {
        byte j = random(i,7);
        byte dummy = nextpieces[i];
        nextpieces[i] = nextpieces[j];
        nextpieces[j] = dummy;
    }
}

byte computeLevel()
{
    if (lines>200) { return 20; }
    return ( ((byte) lines) / 10 );
}
