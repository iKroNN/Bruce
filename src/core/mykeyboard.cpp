#include "mykeyboard.h"
#include "powerSave.h"
#include "sd_functions.h"
#include "modules/ir/TV-B-Gone.h"
#include "modules/rf/rf.h"
#include "modules/others/bad_usb.h"
#include "modules/others/webInterface.h"


#if defined(HAS_TOUCH)
struct box_t
{
  int x;
  int y;
  int w;
  int h;
  std::uint16_t color;
  int touch_id = -1;
  char key;
  char key_sh;

  void clear(void)
  {
    for (int i = 0; i < 8; ++i)
    {
      tft.fillRect(x, y, w, h,BGCOLOR);
    }
  }
  void draw(void)
  {
    int ie = touch_id < 0 ? 4 : 8;
    for (int i = 0; i < ie; ++i)
    {
      tft.drawRect(x, y, w, h,color);
      tft.setTextColor(color);
      tft.drawChar(key,x+w/2-FM*LW/2,y+h/2-FM*LH/2);
    }
  }
  bool contain(int x, int y)
  {
    return this->x <= x && x < (this->x + this->w)
        && this->y <= y && y < (this->y + this->h);
  }
};

static constexpr std::size_t box_count = 52;
static box_t box_list[box_count];

#endif
/* Verifies Upper Btn to go to previous item */

bool checkNextPress(){
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed('/') || Keyboard.isKeyPressed('.'))
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnC.isHolding() || M5.BtnC.isPressed())
  #elif ! defined(HAS_SCREEN)
    // always return false
    if(false)
  #else
    if(digitalRead(DW_BTN)==LOW)
  #endif
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }

  else return false;
}

/* Verifies Down Btn to go to next item */
bool checkPrevPress() {
  #if defined(STICK_C_PLUS)
    if(axp192.GetBtnPress())
  #elif defined(CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(',') || Keyboard.isKeyPressed(';'))
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnA.isHolding() || M5.BtnA.isPressed())
  #elif ! defined(HAS_SCREEN)
    // always return false
    if(false)
  #else 
    if(digitalRead(UP_BTN)==LOW)
  #endif
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }

  else return false;
}

/* Verifies if Select or OK was pressed */
bool checkSelPress(){
  checkPowerSaveTime();
  #if defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed(KEY_ENTER) || digitalRead(0)==LOW)
  #elif ! defined(HAS_SCREEN)
    // always return false
    if(false)
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnB.isHolding() || M5.BtnB.isPressed())
  #else
    if(digitalRead(SEL_BTN)==LOW)
  #endif
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    return true;
  }

  else return false;
}

bool checkEscPress(){
  #if defined(STICK_C_PLUS2)
    if(digitalRead(UP_BTN)==LOW)
  #elif defined(STICK_C_PLUS)
    if(axp192.GetBtnPress())
  #elif defined (CARDPUTER)
    Keyboard.update();
    if(Keyboard.isKeyPressed('`') || Keyboard.isKeyPressed(KEY_BACKSPACE))
  #elif ! defined(HAS_SCREEN)
    // always return false
    if(false)
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnA.isHolding() || M5.BtnA.isPressed())
  #else
    if(digitalRead(UP_BTN)==LOW)
  #endif
  {
    if(wakeUpScreen()){
      delay(200);
      return false;
    }
    returnToMenu=true;
    return true;
  }
  else { return false; }
}

bool checkAnyKeyPress() {
  #if defined (CARDPUTER)   // If any key is pressed, it'll jump the boot screen
    Keyboard.update();
    if(Keyboard.isPressed())
  #elif defined(M5STACK)
    M5.update();
    if(M5.BtnA.isHolding() || M5.BtnA.isPressed() || M5.BtnB.isHolding() || M5.BtnB.isPressed() || M5.BtnC.isHolding() || M5.BtnC.isPressed())    
  #elif ! defined(HAS_SCREEN)
    // always return false
    if(false)
  #else
    if(digitalRead(SEL_BTN)==LOW)  // If M5 key is pressed, it'll jump the boot screen
  #endif
      return true;
  // else
  return false;

}

#ifdef CARDPUTER
void checkShortcutPress(){
  // shortctus to quickly starts apps
    Keyboard.update();
    if(Keyboard.isKeyPressed('i'))  otherIRcodes();
    if(Keyboard.isKeyPressed('r') || Keyboard.isKeyPressed('s'))  otherRFcodes();
    if(Keyboard.isKeyPressed('b'))  usb_setup();  // badusb
    if(Keyboard.isKeyPressed('w'))  loopOptionsWebUi();
    if(Keyboard.isKeyPressed('f'))  { setupSdCard() ? loopSD(SD) : loopSD(LittleFS); }
    if(Keyboard.isKeyPressed('l'))  loopSD(LittleFS);
// TODO: other boards?
// TODO: user-configurable
}

int checkNumberShortcutPress() {
    // shortctus to quickly select options
    Keyboard.update();
    char c;
    for (c = '1'; c <= '9'; c++)
        if(Keyboard.isKeyPressed(c)) return(c - '1');
    // else
    return -1;
}

char checkLetterShortcutPress() {
  // shortctus to quickly select options
  Keyboard.update();
  char c;
  for (c = 'a'; c <= 'z'; c++)
      if(Keyboard.isKeyPressed(c)) return(c);
  for (c = 'A'; c <= 'Z'; c++)
      if(Keyboard.isKeyPressed(c)) return(c);
  // else
  return -1;
}
#endif

/* Starts keyboard to type data */
String keyboard(String mytext, int maxSize, String msg) {

  resetTftDisplay();
  bool caps=false;
  int x=0;
  int y=-1;
  int x2=0;
  int y2=0;
  char keys[4][12][2] = { //4 lines, with 12 characteres, low and high caps
    {
      { '1', '!' },//1
      { '2', '@' },//2
      { '3', '#' },//3
      { '4', '$' },//4
      { '5', '%' },//5
      { '6', '^' },//6
      { '7', '&' },//7
      { '8', '*' },//8
      { '9', '(' },//9
      { '0', ')' },//10
      { '-', '_' },//11
      { '=', '+' } //12
     },
    {
      { 'q', 'Q' },//1
      { 'w', 'W' },//2
      { 'e', 'E' },//3
      { 'r', 'R' },//4
      { 't', 'T' },//5
      { 'y', 'Y' },//6
      { 'u', 'U' },//7
      { 'i', 'I' },//8
      { 'o', 'O' },//9
      { 'p', 'P' },//10
      { '[', '{' },//11
      { ']', '}' } //12
    },
    {
      { 'a', 'A' },//1
      { 's', 'S' },//2
      { 'd', 'D' },//3
      { 'f', 'F' },//4
      { 'g', 'G' },//5
      { 'h', 'H' },//6
      { 'j', 'J' },//7
      { 'k', 'K' },//8
      { 'l', 'L' },//9
      { ';', ':' },//10
      { '"', '\'' },//11
      { '|', '\\' } //12
    },
    {
      { '\\', '|' },//1
      { 'z', 'Z' },//2
      { 'x', 'X' },//3
      { 'c', 'C' },//4
      { 'v', 'V' },//5
      { 'b', 'B' },//6
      { 'n', 'N' },//7
      { 'm', 'M' },//8
      { ',', '<' },//9
      { '.', '>' },//10
      { '?', '/' },//11
      { '/', '/' } //12
    }
  };
  int _x = WIDTH/12;
  int _y = (HEIGHT - 54)/4;
  int _xo = _x/2-3;

#if defined(HAS_TOUCH)
  int k=0;
  for(x2=0; x2<12;x2++) {
    for(y2=0; y2<4; y2++) {
      box_list[k].key=keys[y2][x2][0];
      box_list[k].key_sh=keys[y2][x2][1];
      box_list[k].color = ~BGCOLOR;
      box_list[k].x=x2*_x;
      box_list[k].y=y2*_y+54;
      box_list[k].w=_x;
      box_list[k].h=_y;
      k++;
    }
  }
  // OK
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=0;
  box_list[k].y=0;
  box_list[k].w=53;
  box_list[k].h=22;
  k++;
  // CAP
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=55;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // DEL
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=107;
  box_list[k].y=0;
  box_list[k].w=50;
  box_list[k].h=22;
  k++;
  // SPACE
  box_list[k].key=' ';
  box_list[k].key_sh=' ';
  box_list[k].color = ~BGCOLOR;
  box_list[k].x=159;
  box_list[k].y=0;
  box_list[k].w=WIDTH-164;
  box_list[k].h=22;

  k=0;
  x2=0;
  y2=0;
#endif

  int i=0;
  int j=-1;
  bool redraw=true;
  delay(200);
  int cX =0;
  int cY =0;
  tft.fillScreen(BGCOLOR);
  while(1) {
    if(redraw) {
      tft.setCursor(0,0);
      tft.setTextColor(TFT_WHITE, BGCOLOR);
      tft.setTextSize(FM);

      //Draw the rectangles
      if(y<0) {
        tft.fillRect(0,1,WIDTH,22,BGCOLOR);
        tft.drawRect(7,2,46,20,TFT_WHITE);       // Ok Rectangle
        tft.drawRect(55,2,50,20,TFT_WHITE);      // CAP Rectangle
        tft.drawRect(107,2,50,20,TFT_WHITE);     // DEL Rectangle
        tft.drawRect(159,2,74,20,TFT_WHITE);     // SPACE Rectangle
        tft.drawRect(3,32,WIDTH-3,20,FGCOLOR); // mystring Rectangle


        if(x==0 && y==-1) { tft.setTextColor(BGCOLOR, TFT_WHITE); tft.fillRect(7,2,50,20,TFT_WHITE); }
        else tft.setTextColor(TFT_WHITE, BGCOLOR);
        tft.drawString("OK", 18, 4);


        if(x==1 && y==-1) { tft.setTextColor(BGCOLOR, TFT_WHITE); tft.fillRect(55,2,50,20,TFT_WHITE); }
        else if(caps) { tft.fillRect(55,2,50,20,TFT_DARKGREY); tft.setTextColor(TFT_WHITE, TFT_DARKGREY); }
        else tft.setTextColor(TFT_WHITE, BGCOLOR);
        tft.drawString("CAP", 64, 4);


        if(x==2 && y==-1) { tft.setTextColor(BGCOLOR, TFT_WHITE); tft.fillRect(107,2,50,20,TFT_WHITE); }
        else tft.setTextColor(TFT_WHITE, BGCOLOR);
        tft.drawString("DEL", 115, 4);

        if(x>2 && y==-1) { tft.setTextColor(BGCOLOR, TFT_WHITE); tft.fillRect(159,2,74,20,TFT_WHITE); }
        else tft.setTextColor(TFT_WHITE, BGCOLOR);
        tft.drawString("SPACE", 168, 4);
      }

      tft.setTextSize(FP);
      tft.setTextColor(TFT_WHITE, 0x5AAB);
      tft.drawString(msg.substring(0,38), 3, 24);

      tft.setTextSize(FM);

      // reseta o quadrado do texto
      if (mytext.length() == 19 || mytext.length() == 20 || mytext.length() == 38 || mytext.length() == 39) tft.fillRect(3,32,WIDTH-3,20,BGCOLOR); // mystring Rectangle
      // escreve o texto
      tft.setTextColor(TFT_WHITE);
      if(mytext.length()>19) {
        tft.setTextSize(FP);
        if(mytext.length()>38) {
          tft.drawString(mytext.substring(0,38), 5, 34);
          tft.drawString(mytext.substring(38,mytext.length()), 5, 42);
        }
        else {
          tft.drawString(mytext, 5, 34);
        }
      } else {
        tft.drawString(mytext, 5, 34);
      }
      //desenha o retangulo colorido
      tft.drawRect(3,32,WIDTH-3,20,FGCOLOR); // mystring Rectangle


      tft.setTextColor(TFT_WHITE, BGCOLOR);
      tft.setTextSize(FM);


      for(i=0;i<4;i++) {
        for(j=0;j<12;j++) {
          //use last coordenate to paint only this letter
          if(x2==j && y2==i) { tft.setTextColor(~BGCOLOR, BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,BGCOLOR);}
          /* If selected, change font color and draw Rectangle*/
          if(x==j && y==i) { tft.setTextColor(BGCOLOR, ~BGCOLOR); tft.fillRect(j*_x,i*_y+54,_x,_y,~BGCOLOR);}
          
                    
          /* Print the letters */
          if(!caps) tft.drawChar(keys[i][j][0], (j*_x+_xo), (i*_y+56));
          else tft.drawChar(keys[i][j][1], (j*_x+_xo), (i*_y+56));

          /* Return colors to normal to print the other letters */
          if(x==j && y==i) { tft.setTextColor(~BGCOLOR, BGCOLOR); }
        }
      }
      // save actual key coordenate
      x2=x;
      y2=y;
      redraw = false;
      #if defined(HAS_TOUCH)
      TouchFooter();
      #endif
    }

    //cursor handler
    if(mytext.length()>19) {
      tft.setTextSize(FP);
      if(mytext.length()>38) {
        cY=42;
        cX=5+(mytext.length()-38)*LW;
      }
      else {
        cY=34;
        cX=5+mytext.length()*LW;
      }
    } else {
      cY=34;
      cX=5+mytext.length()*LW*2;
    }

    /* When Select a key in keyboard */
    #if defined (CARDPUTER)

    Keyboard.update();
    if (Keyboard.isPressed()) {
      wakeUpScreen();
      tft.setCursor(cX,cY);
      Keyboard_Class::KeysState status = Keyboard.keysState();
      for (auto i : status.word) {
        if(mytext.length()<maxSize) {
          mytext += i;
          if(mytext.length()!=20 && mytext.length()!=20) tft.print(i);
          cX=tft.getCursorX();
          cY=tft.getCursorY();
          if(mytext.length()==20) redraw = true;
          if(mytext.length()==39) redraw = true;
        }
      }
      if (status.del && mytext.length() > 0) {
        // Handle backspace key
        mytext.remove(mytext.length() - 1);
        int fS=FM;
        if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
        else tft.setTextSize(FM);
        tft.setCursor((cX-fS*LW),cY);
        tft.setTextColor(FGCOLOR,BGCOLOR);
        tft.print(" ");
        tft.setTextColor(TFT_WHITE, 0x5AAB);
        tft.setCursor(cX-fS*LW,cY);
        cX=tft.getCursorX();
        cY=tft.getCursorY();
        if(mytext.length()==19) redraw = true;
        if(mytext.length()==38) redraw = true;
      }
      if (status.enter) {
        break;
      }
      delay(150);
    }
    if(checkSelPress()) break;

    #else

    int z=0;
  #if defined(HAS_TOUCH)
    #if defined(M5STACK)
    M5.update();
    auto t = M5.Touch.getDetail();
    if (t.isPressed() || t.isHolding()) 
    #elif defined(T_DISPLAY_S3)
    if (touch.read())
    #elif defined(CYD)
    if (touch.touched())
    #elif defined(MARAUDERV4)
    TouchPoint t;
    bool touched = tft.getTouch(&t.x, &t.y, 600);

    if(rotation==3) { 
      t.y = (HEIGHT+20)-t.y;
      t.x = WIDTH-t.x;
    }
    if(touched)
    #endif
     {
      #if defined(T_DISPLAY_S3)
        auto t = touch.getPoint(0);
        if(rotation==3) {
          t.x = WIDTH-t.x;
        } else if (rotation==1) {
          t.y = (HEIGHT+20)-t.y;
        }
      #elif defined(CYD)
        auto t = touch.getPointScaled();
        if(rotation==3) { 
          t.y = (HEIGHT+20)-t.y;
          t.x = WIDTH-t.x;
        }
      #endif
      if (box_list[48].contain(t.x, t.y)) { break; }      // Ok
      if (box_list[49].contain(t.x, t.y)) { caps=!caps; tft.fillRect(0,54,WIDTH,HEIGHT-54,BGCOLOR); goto THIS_END; } // CAP
      if (box_list[50].contain(t.x, t.y)) goto DEL;               // DEL
      if (box_list[51].contain(t.x, t.y)) { mytext += box_list[51].key; goto ADD; } // SPACE
      for(k=0;k<48;k++){
        if (box_list[k].contain(t.x, t.y)) {
          if(caps) mytext += box_list[k].key_sh;
          else mytext += box_list[k].key;
        }
      }
      wakeUpScreen();
      THIS_END:
      #if defined(T_DISPLAY_S3)
      t.x=WIDTH+1;
      t.y=HEIGHT+11;
      #endif      
      redraw=true;
    }
    #endif  

    if(checkSelPress())  {
      tft.setCursor(cX,cY);
      if(caps) z=1;
      else z=0;
      if(x==0 && y==-1) break;
      else if(x==1 && y==-1) caps=!caps;
      else if(x==2 && y==-1 && mytext.length() > 0) {
        DEL:
        mytext.remove(mytext.length()-1);
        int fS=FM;
        if(mytext.length()>19) { tft.setTextSize(FP); fS=FP; }
        else tft.setTextSize(FM);
        tft.setCursor((cX-fS*LW),cY);
        tft.setTextColor(FGCOLOR,BGCOLOR);
        tft.print(" ");
        tft.setTextColor(TFT_WHITE, 0x5AAB);
        tft.setCursor(cX-fS*LW,cY);
        cX=tft.getCursorX();
        cY=tft.getCursorY();
      }
      else if(x>2 && y==-1 && mytext.length()<maxSize) mytext += " ";
      else if(y>-1 && mytext.length()<maxSize) {
        ADD:
        mytext += keys[y][x][z];
        if(mytext.length()!=20 && mytext.length()!=20) tft.print(keys[y][x][z]);
        cX=tft.getCursorX();
        cY=tft.getCursorY();
      }
      redraw = true;
      delay(200);
    }

    /* Down Btn to move in X axis (to the right) */
    if(checkNextPress())
    {
      delay(200);
      if(checkNextPress()) { x--; delay(250); } // Long Press
      else x++; // Short Press

      if(y<0 && x>3) x=0;
      if(x>11) x=0;
      else if (x<0) x=11;
      redraw = true;
    }
    /* UP Btn to move in Y axis (Downwards) */
    if(checkPrevPress()) {
      delay(200);

      if(checkPrevPress()) { y--; delay(250);  }// Long press
      else y++; // short press

      if(y>3) { y=-1; }
      else if(y<-1) y=3;
      redraw = true;
    }

    #endif

  }

  //Resets screen when finished writing
  tft.fillRect(0,0,WIDTH,HEIGHT,BGCOLOR);
  resetTftDisplay();

  return mytext;
}

void checkReboot() {
    int countDown;
    #if defined(STICK_C_PLUS2)
        /* Long press power off */
        if (digitalRead(UP_BTN)==LOW)
        {
            uint32_t time_count = millis();
            while (digitalRead(UP_BTN)==LOW)
            {
                // Display poweroff bar only if holding button
                if (millis() - time_count > 500) {
                    tft.setCursor(60, 10);
                    tft.setTextSize(1);
                    tft.setTextColor(TFT_RED, TFT_BLACK);
                    countDown = (millis() - time_count) / 1000 + 1;
                    tft.printf(" PWR OFF IN %d/3\n", countDown);
                    delay(10);
                }
            }
        }
    #endif
}
