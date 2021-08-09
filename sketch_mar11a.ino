/*
 * This is a small hexadecimal "computer" program for the Arduino UNO
 * 
 * It allows you to code on the microcontroller and save your programs
 * through the use of the EEPROM
 * 
 * Diego Villalvazo
 * 2018
 */

 /*
  * Comments:
  * This was one of my first attempts at making something that was
  * programmable and I hope to reprogram this entire thing as I really
  * don't think I knew what I was doing back then.
  */

#include <LiquidCrystal.h>
#include <EEPROM.h>

//LCD setup
const int d0_a = 36, d1_a = 37, d2_a = 38, d3_a = 39, d4_a = 40, d5_a = 41, d6_a = 42, d7_a = 43;
const int d0_b = 25, d1_b = 26, d2_b = 27, d3_b = 28, d4_b = 29, d5_b = 30, d6_b = 31, d7_b = 32;

//a: display a - top
const int rs_a = 33, rw_a = 34, en_a = 35; 
LiquidCrystal lcd_a(rs_a, rw_a, en_a, d0_a, d1_a, d2_a, d3_a, d4_a, d5_a, d6_a, d7_a);
//b: display b - bottom
const int rs_b = 22, rw_b = 23, en_b = 24;
LiquidCrystal lcd_b(rs_b, rw_b, en_b, d0_b, d1_b, d2_b, d3_b, d4_b, d5_b, d6_b, d7_b);

//CONSOLE: tag
String a_dispTitle = ":CONSOLE";

//enable: once set to true it will leave the memory as is, if set to false memory will be cleared
bool enable = false;

//input setup
int in0, in1, mru, mld, alt, rnx, ent, inA, inB, inC, inD, inE, inF;

//cursor position
int cursorPos = 0;

//binary cursor position
int cursorBin = 0;

//hexadecimal memory cursor position
int cursorMem = 0;

//memory
int mainMem[256];
int bytesFree;
int bytesUsed;
int saveSlot = 0;
//saveSlot is A - F and referenced with 0 - 5 respectively

//modes
int mode = 0;

//delays
int delayButton = 150;
int loadDelay = 2;

//user input
int userIn[] = {0, 0, 0, 0, 0, 0, 0, 0};

//specific values
bool terminal_writing = true;

//flags
bool hadError = false;
bool errorFlag = false;
int errorPos;
int errorCode;
String errorCause;

//code
int STR = 1;
int JMP = 2;

int JFA = 10;
int JFB = 11;
int JFC = 12;
int JFD = 13;
int JFE = 14;
int JFF = 15;

int ADD = 16;
int SUB = 17;
int ADA = 18;
int SUA = 19;

int RNX = 30;
int RND = 31;

int WFI = 153;
int SFA = 154;
int SFB = 155;
int SFC = 156;
int SFD = 157;
int SFE = 158;
int SFF = 159;

int IFZ = 160;
int IFX = 161;

int ADT = 167;
int AD1 = 168;
int SUT = 169;
int SU1 = 170;

int SEA = 171;
int SET = 172;
int SEF = 173;
int CLR = 174;
int CLF = 175;

int SHR = 176;
int SHL = 177;
int SR1 = 178;
int SL1 = 179;

int MVT = 190;
int SWT = 191;

int PSX = 208;

int CLC = 224;
int CCL = 225;
int CCC = 226;
int DVH = 227;
int DVD = 228;
int DAH = 229;
int DAD = 230;
int FCL = 231;
int FCC = 232;
int FLR = 233;
int FCR = 234;

int FLC = 238;
int DSV = 239;

int HLT = 255;

//misc
int cb = 0; //clipboard

void setup()
{
  lcd_a.begin(20, 4);
  lcd_b.begin(20, 4);

in0, in1, mru, mld, alt, rnx, ent, inA, inB, inC, inD, inE, inF;

  pinMode(1, INPUT);  //rnx
  
  pinMode(2, INPUT);  //inD
  pinMode(3, INPUT);  //inE
  pinMode(4, INPUT);  //inF
  pinMode(5, INPUT);  //inA
  pinMode(6, INPUT);  //inB
  pinMode(7, INPUT);  //inC
  
  pinMode(8, INPUT);  //ent
  pinMode(9, INPUT);  //in1
  pinMode(10, INPUT); //in0
  pinMode(11, INPUT); //alt
  pinMode(12, INPUT); //mru
  pinMode(13, INPUT); //mld

  lcd_a.clear();
  lcd_b.clear();
}

void loop()
{
  if (!enable)
  {
    lcd_a.setCursor(0, 0);
    lcd_a.print("INITIALIZING...");
    lcd_a.setCursor(4, 1);
    lcd_a.print("OF 255 ");
    for (int m = 0; m < 255; m++)
    {
      mainMem[m] = 0, HEX;
      lcd_a.setCursor(0, 1);
      lcd_a.print(m);
      delay(10);
    }
    lcd_a.setCursor(0, 1);
    lcd_a.print(255);
    enable = true;
    lcd_a.clear();
  }
  else
  {
    rnx = digitalRead(1);
    inD = digitalRead(2);
    inE = digitalRead(3);
    inF = digitalRead(4);
    inA = digitalRead(5);
    inB = digitalRead(6);
    inC = digitalRead(7);
    ent = digitalRead(8);
    in1 = digitalRead(9);
    in0 = digitalRead(10);
    alt = digitalRead(11);
    mru = digitalRead(12);
    mld = digitalRead(13);

    switch(mode)
    {
      case 0: //TERMINAL
        terminal();
        break;
      case 1: //MEMORY
        memory();
        break;
      case 2: //RESTRICTION-!
        mode = 0;
        terminal_writing = true;
        cursorMem = 0;
        break;
    }
  }
}

void terminal()
{
  if(terminal_writing)
  {
    int valHEX = (userIn[0] * 128) + (userIn[1] * 64) + (userIn[2] * 32) + (userIn[3] * 16) + (userIn[4] * 8) + (userIn[5] * 4) + (userIn[6] * 2) + (userIn[7] * 1);
    
    //draw UI
    lcd_b.setCursor(0, 0);
    lcd_b.print(":TERMINAL");
    lcd_b.setCursor(10, 0);
    lcd_b.print("X");
    lcd_b.setCursor(11, 0);
    lcd_b.print(cursorPos);
    lcd_b.setCursor(13, 0);
    lcd_b.print("$");
    lcd_b.setCursor(14, 0);
    lcd_b.print(cursorMem, HEX);
    lcd_b.setCursor(10, 1);
    lcd_b.print("=");
    lcd_b.setCursor(11, 1);
    lcd_b.print(valHEX, HEX);
    lcd_b.setCursor(13, 1);
    lcd_b.print("#");
    lcd_b.setCursor(14, 1);
    lcd_b.print(mainMem[cursorMem], HEX);
    lcd_b.setCursor(0, 1);
    lcd_b.print(">");
    lcd_b.setCursor(10,2);
    lcd_b.print(":");

    //controls
    //print memory byte
    for (int i = 0; i < 8; i++)
    {
      lcd_b.setCursor(1 + i, 1);
      lcd_b.print(userIn[i]);
    }
    //input check
    if (ent)
    {
      delay(delayButton);
      mainMem[cursorMem] = valHEX;
      for (int i = 0; i < 8; i++)
      {
        userIn[i] = 0;
      }
      cursorPos = 0;
      cursorMem++;
    }
    if (in0)
    {
      userIn[cursorPos] = 0;
      delay(delayButton);
      cursorPos++;
    }
    if (in1)
    {
      userIn[cursorPos] = 1;
      delay(delayButton);
      cursorPos++;
    }
    if (mru)
    {
      delay(delayButton);
      cursorMem++;
      cursorPos = 0;
    }
    if (mld)
    {
      delay(delayButton);
      cursorMem--;
      cursorPos = 0;
    }
    if (mru && alt)
    {
      delay(delayButton);
      cursorMem = cursorMem + 15;
      cursorPos = 0;
    }
    if (mld && alt)
    {
      delay(delayButton);
      cursorMem = cursorMem - 15;
      cursorPos = 0;
    }
    
    if (inA && alt)
    {
      delay(delayButton);
      cb = mainMem[cursorMem];
      lcd_b.setCursor(11,2);
      lcd_b.print(cb, HEX);
    }
    if (inB && alt)
    {
      delay(delayButton);
      mainMem[cursorMem] = cb;
    }
    /*if (inC)
    {
      delay(delayButton);
      for (int i = 0; i < 8; i++)
      {
        userIn[i] = 0;
      }
      cursorPos = 0;
      mainMem[cursorMem] = 0;
    }*/
    if (inC && alt)
    {
      delay(delayButton);
      mainMem[cursorMem] = 0;
    }
    if (inD && alt)
    {
      delay(delayButton);
      cursorMem = valHEX;
      for (int i = 0; i < 8; i++)
      {
        userIn[i] = 0;
      }
      cursorPos = 0;
    }
    if (inE && alt)
    {
      delay(delayButton);
    }
    if (inF && alt)
    {
      delay(delayButton);
    }

    if (rnx)
    {
    if(hadError) lcd_a.clear(), hadError = false;
    delay(delayButton);
    terminal_writing = false;
    }
    
    //mode change
    if (alt && rnx)
    {
      delay(delayButton);
      mode++;
      lcd_b.clear();
    }

    //restrictions
    if (cursorPos > 7)
    {
      cursorPos = 0;
    }
    if (cursorMem > 255)
    {
      cursorMem = 0;
    }
    if (cursorMem < 0)
    {
      cursorMem = 255;
    }

    //drawing restrictions
    if (cursorMem < 16)
    {
      lcd_b.setCursor(15, 0);
      lcd_b.print(" ");
    }
    if (valHEX < 16)
    {
      lcd_b.setCursor(12, 1);
      lcd_b.print(" ");
    }
    if (mainMem[cursorMem] < 16)
    {
      lcd_b.setCursor(15, 1);
      lcd_b.print(" ");
    }
    if (cb < 16)
    {
      lcd_b.setCursor(12, 2);
      lcd_b.print(" ");
    }
  }
  else
  {
    bool flagSTR = false;
    int startCodePos;
    cursorPos = 0;
    cursorBin = 0;
    if (!flagSTR)
    {
      for (cursorMem = 0; cursorMem < 256; cursorMem++)
      {
        //testing for code 01
        if (mainMem[cursorMem] == STR)
        {
          startCodePos = cursorMem;
          flagSTR = true;
          break;
        }
      }
    }
    if (flagSTR)
    {
      for (cursorMem = startCodePos; cursorMem < 256; cursorMem++)
      {
        //reading code
        if (mainMem[cursorMem] == JMP)
        {
          int target;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem = target;
        }
        
        //------------------------------------------------------------------------------

        if (mainMem[cursorMem] == JFA)
        {
          inA = digitalRead(5);
          if(inA)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        if (mainMem[cursorMem] == JFB)
        {
          inB = digitalRead(6);
          if(inB)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        if (mainMem[cursorMem] == JFC)
        {
          inC = digitalRead(7);
          if(inC)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        if (mainMem[cursorMem] == JFD)
        {
          inD = digitalRead(2);
          if(inD)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        if (mainMem[cursorMem] == JFE)
        {
          inE = digitalRead(3);
          if(inE)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        if (mainMem[cursorMem] == JFF)
        {
          inF = digitalRead(4);
          if(inF)
          {
            int target;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem = target;
          }
          else
          {
            cursorMem += 2;
          }
        }
        
        //------------------------------------------------------------------------------
        
        if (mainMem[cursorMem] == ADD)
        {
          int opX, opY, result, target;
          cursorMem++;
          opX = mainMem[cursorMem];
          cursorMem++;
          opY = mainMem[cursorMem];
          result = opX + opY;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = result;
        }
        if (mainMem[cursorMem] == SUB)
        {
          int opX, opY, result, target;
          cursorMem++;
          opX = mainMem[cursorMem];
          cursorMem++;
          opY = mainMem[cursorMem];
          result = opX - opY;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = result;
        }
        if (mainMem[cursorMem] == ADA)
        {
          int opX, opY, result, returnTarget, target;
          cursorMem++;
          returnTarget = cursorMem;
          cursorMem = mainMem[cursorMem];
          opX =  mainMem[cursorMem];

          cursorMem = returnTarget;
          cursorMem++;
          returnTarget = cursorMem;
          cursorMem = mainMem[cursorMem];
          opY =  mainMem[cursorMem];

          cursorMem = returnTarget;

          result = opX + opY;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = result;
        }
        if (mainMem[cursorMem] == SUA)
        {
          int opX, opY, result, returnTarget, target;
          cursorMem++;
          returnTarget = cursorMem;
          cursorMem = mainMem[cursorMem];
          opX =  mainMem[cursorMem];

          cursorMem = returnTarget;
          cursorMem++;
          returnTarget = cursorMem;
          cursorMem = mainMem[cursorMem];
          opY =  mainMem[cursorMem];

          cursorMem = returnTarget;

          result = opX - opY;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = result;
        }
        if (mainMem[cursorMem] == RNX)
        {
          int limitX, limitY, target, randVal;
          cursorMem++;
          limitX = mainMem[cursorMem];
          cursorMem++;
          limitY = mainMem[cursorMem];
          cursorMem++;
          target = mainMem[cursorMem];
          randVal = round(random(limitX, limitY));
          mainMem[target] = randVal;
        }
        if (mainMem[cursorMem] == RND)
        {
          int target, randVal;
          cursorMem++;
          target = mainMem[cursorMem];
          randVal = round(random(0, 255));
          mainMem[target] = randVal;
        }
        if (mainMem[cursorMem] == WFI)
        {
          while(digitalRead(2) == LOW && digitalRead(3) == LOW && digitalRead(4) == LOW && digitalRead(5) == LOW && digitalRead(6) == LOW && digitalRead(7) == LOW)
          {
            //do nothing
          }
        }        
        if (mainMem[cursorMem] == SFA)
        {
          inA = digitalRead(5);
          if(inA)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == SFB)
        {
          inB = digitalRead(6);
          if(inB)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == SFC)
        {
          inC = digitalRead(7);
          if(inC)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == SFD)
        {
          inD = digitalRead(2);
          if(inD)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == SFE)
        {
          inE = digitalRead(3);
          if(inE)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == SFF)
        {
          inF = digitalRead(4);
          if(inF)
          {
            int target, val;
            cursorMem++;
            target = mainMem[cursorMem];
            cursorMem++;
            val = mainMem[cursorMem];
            mainMem[target] = val;
            cursorMem++;
          }
          else
          {
            cursorMem += 3;
          }
        }
        if (mainMem[cursorMem] == IFZ)
        {
          int compAdd, targetX, targetY;
          cursorMem++;
          compAdd = mainMem[cursorMem];
          cursorMem++;
          targetX = mainMem[cursorMem];
          cursorMem++;
          targetY = mainMem[cursorMem];
          if (mainMem[compAdd] == 0)
          {
            cursorMem = targetX - 1;
          }
          else
          {
            cursorMem = targetY - 1;
          }
        }
        if (mainMem[cursorMem] == IFX)
        {
          int compAdd, compVal, targetX, targetY;
          cursorMem++;
          compAdd = mainMem[cursorMem];
          cursorMem++;
          compVal = mainMem[cursorMem];
          cursorMem++;
          targetX = mainMem[cursorMem];
          cursorMem++;
          targetY = mainMem[cursorMem];
          if (mainMem[compAdd] == compVal)
          {
            cursorMem = targetX - 1;
          }
          else
          {
            cursorMem = targetY - 1;
          }
        }
        if (mainMem[cursorMem] == ADT)
        {
          int target, addValue;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          addValue = mainMem[cursorMem];
          mainMem[target] = mainMem[target] + addValue;
        }
        if (mainMem[cursorMem] == AD1)
        {
          int target;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = mainMem[target] + 1;
        }
        if (mainMem[cursorMem] == SUT)
        {
          int target, subValue;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          subValue = mainMem[cursorMem];
          mainMem[target] = mainMem[target] - subValue;
        }
        if (mainMem[cursorMem] == SU1)
        {
          int target;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = mainMem[target] - 1;
        }
        if (mainMem[cursorMem] == SEA)
        {
          int target, toValue;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          toValue = mainMem[cursorMem];
          mainMem[target] = mainMem[toValue];

        }
        if (mainMem[cursorMem] == SET)
        {
          int target, setValue;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          setValue = mainMem[cursorMem];
          mainMem[target] = setValue;
        }
        if (mainMem[cursorMem] == SEF)
        {
          int targetX, targetY, value, loops;
          cursorMem++;
          targetX = mainMem[cursorMem];
          cursorMem++;
          targetY = mainMem[cursorMem];
          cursorMem++;
          value = mainMem[cursorMem];
          cursorMem++;
          for (loops = 0; loops <= targetY - targetX; loops++)
          {
            mainMem[targetX + loops] = value;
          }
        }
        if (mainMem[cursorMem] == CLR)
        {
          int target;
          cursorMem++;
          target = mainMem[cursorMem];
          mainMem[target] = 0;
        }
        if (mainMem[cursorMem] == CLF)
        {
          int targetX, targetY, loops;
          cursorMem++;
          targetX = mainMem[cursorMem];
          cursorMem++;
          targetY = mainMem[cursorMem];
          cursorMem++;
          for (loops = 0; loops <= targetY - targetX; loops++)
          {
            mainMem[targetX + loops] = 0;
          }
        }
        if (mainMem[cursorMem] == SHR)
        {
          int target, places, savedByte;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          places = mainMem[cursorMem];
          savedByte = mainMem[target];
          mainMem[target] = 0;
          if(target + places < 255)
          {
            mainMem[target + places] = savedByte;
          }
          else
          {
            mainMem[(target + places) - 256] = savedByte;
          }
        }
        if (mainMem[cursorMem] == SHL)
        {
          int target, places, savedByte;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          places = mainMem[cursorMem];
          savedByte = mainMem[target];
          mainMem[target] = 0;
          if(target - places < 0)
          {
            mainMem[256 - (target + places)] = savedByte;
          }
          else
          {
            mainMem[target - places] = savedByte;
          }
        }
        if (mainMem[cursorMem] == SR1)
        {
          int target, savedByte;
          cursorMem++;
          target = mainMem[cursorMem];
          savedByte = mainMem[target];
          mainMem[target] = 0;
          mainMem[target + 1] = savedByte;
        }
        if (mainMem[cursorMem] == SL1)
        {
          int target, savedByte;
          cursorMem++;
          target = mainMem[cursorMem];
          savedByte = mainMem[target];
          mainMem[target] = 0;
          mainMem[target - 1] = savedByte;
        }
        if (mainMem[cursorMem] == MVT)
        {
          int target, destination;
          cursorMem++;
          target = mainMem[cursorMem];
          cursorMem++;
          destination = mainMem[cursorMem];
          mainMem[destination] = mainMem[target];
          mainMem[target] = 0;
          
        }
        if (mainMem[cursorMem] == SWT)
        {
          int targetX, targetY, saveX, saveY;
          cursorMem++;
          targetX = mainMem[cursorMem];
          saveX = mainMem[targetX];
          cursorMem++;
          targetY = mainMem[cursorMem];
          saveY = mainMem[targetY];
          mainMem[targetX] = saveY;
          mainMem[targetY] = saveX;
        }
        if (mainMem[cursorMem] == PSX)
        {
          int sec;
          cursorMem++;
          sec = mainMem[cursorMem];
          delay(sec * 100);
        }
        if (mainMem[cursorMem] == CLC)
        {
          lcd_a.clear();
        }
        if (mainMem[cursorMem] == CCL)
        {
          int lineNum;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          //test that line val is less than or equal to 3
          if(lineNum <= 3)
          {
            for(int i = 0; i <= 20; i++)
            {
              lcd_a.setCursor(i, lineNum);
              lcd_a.print(" ");
            }
            errorFlag = false;
          }
          else
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
          }
        }
        if (mainMem[cursorMem] == CCC)
        {
          int columnNum;
          cursorMem++;
          columnNum = mainMem[cursorMem];
          //test that line val is less than or equal to 3
          if(columnNum <= 20)
          {
            for(int i = 0; i <= 3; i++)
            {
              lcd_a.setCursor(columnNum, i);
              lcd_a.print(" ");
            }
            errorFlag = false;
          }
          else
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
          }
        }
        if (mainMem[cursorMem] == DVH)
        {
          int lineNum, columnNum, valHex;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          cursorMem++;
          columnNum = mainMem[cursorMem];
          cursorMem++;
          valHex = mainMem[cursorMem];
          lcd_a.setCursor(columnNum, lineNum);
          lcd_a.print(valHex, HEX);
        }
        if (mainMem[cursorMem] == DVD)
        {
          int lineNum, columnNum, valDec;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          cursorMem++;
          columnNum = mainMem[cursorMem];
          cursorMem++;
          valDec = mainMem[cursorMem];
          lcd_a.setCursor(columnNum, lineNum);
          lcd_a.print(valDec);
        }
        if (mainMem[cursorMem] == DAH)
        {
          int lineNum, columnNum, targetAddress, valHex;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          cursorMem++;
          columnNum = mainMem[cursorMem];
          cursorMem++;
          targetAddress = mainMem[cursorMem];
          valHex = mainMem[targetAddress];
          lcd_a.setCursor(columnNum, lineNum);
          lcd_a.print(valHex, HEX);
        }
        if (mainMem[cursorMem] == DAD)
        {
          int lineNum, columnNum, targetAddress, valDec;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          cursorMem++;
          columnNum = mainMem[cursorMem];
          cursorMem++;
          targetAddress = mainMem[cursorMem];
          valDec = mainMem[targetAddress];
          lcd_a.setCursor(columnNum, lineNum);
          lcd_a.print(valDec);
        }
        if (mainMem[cursorMem] == FCL)
        {
          int lineNum, valHex;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          cursorMem++;
          valHex = mainMem[cursorMem];
          if(lineNum <= 3)
          {
            for(int i = 0; i < 20; i++)
            {
              lcd_a.setCursor(i, lineNum);
              lcd_a.print(valHex);
            }
            errorFlag = false;
          }
          else
          {
            errorFlag = true;
            errorPos = --cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
          }
        }
        if (mainMem[cursorMem] == FCC)
        {
          int columnNum, valHex;
          cursorMem++;
          columnNum = mainMem[cursorMem];
          cursorMem++;
          valHex = mainMem[cursorMem];
          if(columnNum < 20)
          {
            for(int i = 0; i < 4; i++)
            {
              lcd_a.setCursor(columnNum, i);
              lcd_a.print(valHex);
            }
            errorFlag = false;
          }
          else
          {
            errorFlag = true;
            errorPos = --cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
          }
        }
        if (mainMem[cursorMem] == FLR)
        {
          int lineNum, val_0, val_1, valHex;
          cursorMem++;
          lineNum = mainMem[cursorMem];
          if(lineNum > 3)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          val_0 = mainMem[cursorMem];
          if(val_0 > 19)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          val_1 = mainMem[cursorMem];
          if(val_1 > 19)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          valHex = mainMem[cursorMem];
          for(int i = val_0; i <= val_1; i++)
          {
            lcd_a.setCursor(i, lineNum);
            lcd_a.print(valHex);
          }
          errorFlag = false;
        }
        if (mainMem[cursorMem] == FCR)
        {
          int columnNum, val_0, val_1, valHex;
          cursorMem++;
          columnNum = mainMem[cursorMem];
          if(columnNum > 19)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          val_0 = mainMem[cursorMem];
          if(val_0 > 3)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          val_1 = mainMem[cursorMem];
          if(val_1 > 3)
          {
            errorFlag = true;
            errorPos = cursorMem;
            errorCode = 01;
            errorCause = "Par out of range";
            error();
            break;
          }
          cursorMem++;
          valHex = mainMem[cursorMem];
          for(int i = val_0; i <= val_1; i++)
          {
            lcd_a.setCursor(columnNum, i);
            lcd_a.print(valHex);
          }
          errorFlag = false;
        }
        //----------------------------------------------------------------------------------
        if (mainMem[cursorMem] == FLC)
        {
          int val;
          cursorMem++;
          val = mainMem[cursorMem];
          for(int i = 0; i < 4; i++)
          {
            for(int j = 0; j < 20; j++)
            {
              lcd_a.setCursor(j,i);
              lcd_a.print(val, HEX);
            }
          }
        }
        if (mainMem[cursorMem] == DSV)
        {
        }
        //----------------------------------------------------------------------------------
        
        if (mainMem[cursorMem] == HLT)
        {
          break;
        }

      }
      terminal_writing = true;
    }
  }
  console();
}

void console()
{
  //draw UI
  lcd_a.setCursor(0, 0);
  if(!errorFlag)
  {
    lcd_a.print(a_dispTitle);
  }
  else
  {
    lcd_a.print("!ERROR  ");
  }
}

void error()
{
  //draw UI
  //lcd_a.setCursor(0, 0);
  lcd_a.clear();
  //lcd_a.print("!ERROR");
  lcd_a.setCursor(0, 1);
  lcd_a.print("-$:");
  lcd_a.setCursor(3, 1);
  lcd_a.print(errorPos);
  lcd_a.setCursor(0, 2);
  lcd_a.print("-#:");
  lcd_a.setCursor(3, 2);
  lcd_a.print(errorCode);
  lcd_a.setCursor(0, 3);
  lcd_a.print("-?:");
  lcd_a.setCursor(3, 3);
  lcd_a.print(errorCause);

  hadError = true;

  //terminal();
}

void memory()
{
  //draw UI
  lcd_b.setCursor(0, 0);
  lcd_b.print(":MEMORY");
  lcd_b.setCursor(8, 0);
  lcd_b.print(bytesUsed);
  lcd_b.setCursor(11, 0);
  lcd_b.print(":");
  lcd_b.setCursor(12, 0);
  lcd_b.print(bytesFree);
  lcd_b.setCursor(0, 1);
  lcd_b.print(">1:SAVE 0:LOAD");
  lcd_b.setCursor(0, 2);
  lcd_b.print(">SLOT: ");
  lcd_b.setCursor(6, 2);
  lcd_b.print(saveSlot);

  //slot selection----------------------------------------------------------------------------------
  const int MULT = 256;
  
  if(inA) saveSlot = 0;
  if(inB) saveSlot = 1;
  if(inC && !(alt)) saveSlot = 2;
  if(inD) saveSlot = 3;
  if(inE) saveSlot = 4;
  if(inF) saveSlot = 5;

  bytesFree = 256;
  bytesUsed = 0;
  for (int i = saveSlot * MULT; i < (saveSlot * MULT) + 256; i++)
  {
    if (EEPROM.read(i) != 0)
    {
      bytesUsed++;
      bytesFree--;
    }
   }
  //slot selection----------------------------------------------------------------------------------

  //load
  if(in0)
  {
    lcd_b.clear();
    for(int i = 0; i < 256; i++)
    {
      int j = (saveSlot * MULT) + i;
      lcd_b.setCursor(0, 0);
      lcd_b.print("LOADING BYTES");
      lcd_b.setCursor(0, 1);
      lcd_b.print(i);
      lcd_b.setCursor(3, 1);
      lcd_b.print(" OF 255");
      mainMem[i] = EEPROM.read(j);
      delay(loadDelay);
    }
    lcd_b.setCursor(7, 0);
    lcd_b.print(" ");
  }

  //save
  if(in1)
  {
    lcd_b.clear();
    for(int i = 0; i < 256; i++)
    {
      int j = (saveSlot * MULT) + i;
      lcd_b.setCursor(0, 0);
      lcd_b.print("SAVING BYTES");
      lcd_b.setCursor(0, 1);
      lcd_b.print(i);
      lcd_b.setCursor(3, 1);
      lcd_b.print(" OF 256");
      EEPROM.write(j, mainMem[i]);
      delay(loadDelay);
    }
    lcd_b.setCursor(7, 0);
    lcd_b.print(" ");
  }

  //del
  if(inC && alt)
  {
    lcd_b.clear();
    for (int i = saveSlot * MULT; i < (saveSlot * MULT) + 256; i++)
    {
      lcd_b.setCursor(0, 0);
      lcd_b.print("DELETING BYTES");
      lcd_b.setCursor(0, 1);
      lcd_b.print(i);
      lcd_b.setCursor(3, 1);
      lcd_b.print(" | SLOT ");
      lcd_b.setCursor(11, 1);
      lcd_b.print(saveSlot);
      EEPROM.write(i, 0);
      delay(loadDelay);
    }
      lcd_b.setCursor(7, 0);
      lcd_b.print(" ");
  }
  

  //mode change
  if (alt && rnx)
  {
    delay(delayButton);
    mode++;
    lcd_b.clear();
  }

  //drawing limitations
  if(bytesUsed < 10)
  {
    lcd_b.setCursor(9, 0);
    lcd_b.print("  ");
  }
  else if(bytesUsed < 100) 
  {
    lcd_b.setCursor(10, 0);
    lcd_b.print(" ");
  }

  if(bytesFree < 10)
  {
    lcd_b.setCursor(13, 0);
    lcd_b.print("  ");
  }
  else if(bytesFree < 100) 
  {
    lcd_b.setCursor(14, 0);
    lcd_b.print(" ");
  }
}
