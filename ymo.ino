#include <Button.h>
#include <TimerOne.h>
#include <PS2Keyboard.h>
const int DataPin = 2;
const int IRQpin =  3;
PS2Keyboard keyboard;


long calctempodelay = 0;
long tempodelay = 500;
long curtimestamp = 0;
long prevtimestamp = 0;
long lasttickstamp =0;
long wheelfactor = 6; // adjust this to modify the pulseperiod to tempo ratio (wheel radius), higher = faster
int  tickflag = 0; // indicate if time tick has passed

const int bicyclesetup = 0;
const int pot1setup = 1;
const int pot2setup = 1;
const int button3Pin = 4;
const int button4Pin = 5;
const int dreamblaster_enable_pin = 6;
int potentiopin = 2;
long potentiovalue = 0;
int buttonState = 0; 
int prev_buttonState = 0; 
int enabledrumming = 0;
int enabledrumming2 = 0;
int enablebassline = 0;
int enablebassline2 = 0;
int enablebassline3 = 0;
int enablebassline4 = 0;
int enablebassline5 = 0;
int enablebassline6 = 0;
int enablebassline7 = 0;
int enablesynthline2 = 0;
int enablesynthline3 = 0;
int enablesynthline4 = 0;
int enablesynthline5 = 0;
int enablesynthline6 = 0;
int enablesynthline7 = 0;
int enablesynthline = 0;
long seq_poscnt = 0;
byte resonantchannel= 1;
byte channel1_prog = 90;
byte channel2_prog = 33;
byte bassvolume = 0x70;
byte drumsvolume = 0x70;
byte synthvolume = 0x50;
byte leadvolume  = 0x70;

#define _BASSDRUM_NOTE 0x24
#define _SNAREDRUM_NOTE 0x26
#define _CLOSEDHIHAT_NOTE 0x2A
#define _PEDALHIHAT_NOTE 0x2C
#define _OPENHIHAT_NOTE 0x2E
#define _CYMBAL_NOTE 0x31
#define _DRUMPRESS_STATUSCODE 0x99
#define _DRUMRELEASE_STATUSCODE 0x89
#define _SYNTHPRESS_STATUSCODE 0x90
#define _SYNTHRELEASE_STATUSCODE 0x80

void midiwrite(int cmd, int pitch, int velocity) {

  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);

}


void midisetup_sam2195_nrpn_send(int channel, int control1,int control2, int value)
{
   Serial.write((byte)(0xB0+channel));
   Serial.write(0x63);
   Serial.write((byte)control1);
   Serial.write(0x62);
   Serial.write((byte)control2);
   Serial.write(0x06);
   Serial.write((byte)value);
}

void midisetup_sam2195_sysexcfg_send(int channelprefix,int channel, int control, int value)
{
   Serial.write(0xF0);
   Serial.write(0x41);
   Serial.write(0x00);
   Serial.write(0x42);
   Serial.write(0x12);
   Serial.write(0x40);
   Serial.write((byte)(channelprefix*16+channel));   
   Serial.write((byte)control);
   Serial.write((byte)value);   
   Serial.write(0x00);   
   Serial.write(0xF7);   
   
}

void midisetup_sam2195_gsreset(void)
{
  midisetup_sam2195_sysexcfg_send(0,0,0x7F,0);
}

void midiprogchange(int cmd, int prog) {
  Serial.write(cmd);
  Serial.write(prog);
}


Button button3 = Button(button3Pin,BUTTON_PULLDOWN);
Button button4 = Button(button4Pin,BUTTON_PULLDOWN);

/*
 * basssubseq simplification
 * 
 * desired behaviour:
 * a list of notes which are play one by one depending on relativpos
 * a note should also be turned off before the next note is played
 * 
 * is that correct?
 * coding it by hand works of course, but is error prone.
 * you can forget or mistype turning the note off, etc.
 * 
 * if you make an array of notes (the bassline),
 * and use that in a function more generically, you end up with way less code,
 * and more flexibility in trying out new basslines easily.
 * 
 * plus: - less errors, because less typing
 *       - less work, because less typing
 *       
 */

int bassline[] = {34,0,46,0,34,0,46,0,33,0,45,0,33,0,45,0,38,0,50,0,40,0,52,0,41,0,53,0,38,0,50,0,31,0,31,0,33,0,0,0,38,0,36,0,33,0,31,0,31,0,38,0,36,0,43,0,41,0,0,0,43,0,45,0,46,0,46,0,41,0,46,0,45,0,45,0,40,0,45,0,43,0,0,43,41,0,42,0,43,0,0,0,43,0,0,0,45,0,0,45,43,0,44,0,45,0,0,0,33,0,0,0,0,0,33,0,33,0,33,0,33,0,45,0,33,0,45,0}; // define the bassline here


long lastRelativPos = -1; // keep the last relativpos, so we can turn the note off

void basssubseq(long relativpos) {

  // turn off note before
  if (lastRelativPos != -1) {
    midiwrite(0x81, bassline[lastRelativPos], 0x00);
  }

  // play the note
  midiwrite(0x91, bassline[relativpos], bassvolume);

  // set the lastRelativpos
  lastRelativPos = relativpos;
}


int bassline2[] = {40,40,52,40,40,52,40,40,52,40,40,52,40,40,52,43,43,43,55,43,43,55,43,43,55,43,43,55,43,43,55,45,45,45,57,45,45,57,45,45,57,45,45,57,45,45,57,41,41,41,53,41,41,53,40,40,52,40,40,52,38,38,50,38}; // define the bassline here

void basssubseq3(long relativpos) {

  // turn off note before
  if (lastRelativPos != -1) {
    midiwrite(0x81, bassline2[lastRelativPos], 0x00);
  }

  // play the note
  midiwrite(0x91, bassline2[relativpos], bassvolume);

  // set the lastRelativpos
  lastRelativPos = relativpos;
}

void basssubseq2(long relativpos) 
{
  
  switch(relativpos)
    {
    
      case 0:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
      case 4:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 6:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
      case 8:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 10:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 12:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 14:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
       case 18:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
       case 20:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
      case 22:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 24:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
       case 26:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 28:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 30:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
       case 32:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 34:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 36:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 54, bassvolume);
      break;
       case 38:
      midiwrite(0x81, 54, 0x00); 
      midiwrite(0x91, 55, bassvolume);
      break;
      case 42:
      midiwrite(0x81, 55, 0x00); 
      midiwrite(0x91, 54, bassvolume);
      break;
       case 44:
      midiwrite(0x81, 54, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 46:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 55, bassvolume);
      break;
      case 48:
      midiwrite(0x81, 55, 0x00); 
      midiwrite(0x91, 54, bassvolume);
      break;
      case 52:
      midiwrite(0x81, 54, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 54:
      midiwrite(0x81, 49, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 56:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 60:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 62:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 64:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 53, bassvolume);
      break;
      case 66:
      midiwrite(0x81, 53, 0x00); 
      midiwrite(0x91, 55, bassvolume);
      break;
      case 68:
      midiwrite(0x81, 55, 0x00); 
      midiwrite(0x91, 57, bassvolume);
      break;
      case 70:
      midiwrite(0x81, 57, 0x00); 
      midiwrite(0x91, 58, bassvolume);
      break;
      case 74:
      midiwrite(0x81, 58, 0x00); 
      midiwrite(0x91, 57, bassvolume);
      break;
      case 76:
      midiwrite(0x81, 57, 0x00); 
      midiwrite(0x91, 55, bassvolume);
      break;
      case 78:
      midiwrite(0x81, 55, 0x00); 
      midiwrite(0x91, 58, bassvolume);
      break;
      case 80:
      midiwrite(0x81, 58, 0x00); 
      midiwrite(0x91, 57, bassvolume);
      break;
      case 84:
      midiwrite(0x81, 57, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 86:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 53, bassvolume);
      break;
      case 88:
      midiwrite(0x81, 53, 0x00); 
      midiwrite(0x91, 55, bassvolume);
      break;
      case 92:
      midiwrite(0x81, 55, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 94:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 96:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 53, bassvolume);
      break;
      case 98:
      midiwrite(0x81, 53, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 102:
      midiwrite(0x81, 48, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 104:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 108:
      midiwrite(0x81, 52, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 112:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 116:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 120:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 122:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 124:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 126:
      midiwrite(0x81, 48, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
    
          
    default :
      break;
    }    
}


void basssubseq4(long relativpos) 
{
  
  switch(relativpos)
    {
    
      case 2:
      midiwrite(0x81, 63, 0x00); 
      midiwrite(0x91, 51, bassvolume);
      break;
      case 6:
      midiwrite(0x91, 51, bassvolume);
      break;
      case 10:
      midiwrite(0x81, 51, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
      case 14:
      midiwrite(0x91, 45, bassvolume);
      break;
       case 18:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
       case 22:
      midiwrite(0x91, 44, bassvolume);
      break;
      case 26:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
      case 30: 
      midiwrite(0x91, 43, bassvolume);
      break;
       case 34:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 42, bassvolume);
      break;
      case 38:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 40:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
       case 41:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 42:
      midiwrite(0x91, 48, bassvolume);
      break;
      case 46:
      midiwrite(0x91, 48, bassvolume);
      break;
       case 50:
      midiwrite(0x81, 48, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 54:
      midiwrite(0x91, 47, bassvolume);
      break;
       case 58:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 62:
      midiwrite(0x91, 46, bassvolume);
      break;
      case 66:
      midiwrite(0x81, 46, 0x00); 
      midiwrite(0x91, 45, bassvolume);
      break;
      case 70:
      midiwrite(0x91, 45, bassvolume);
      break;
      case 72:
      midiwrite(0x81, 45, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 73:
      midiwrite(0x81, 50, 0x00); 
      break;
      case 74:
      midiwrite(0x91, 50, bassvolume);
      break;
      case 78:
      midiwrite(0x81, 50, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 80:
      midiwrite(0x81, 49, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 81:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 82:
      midiwrite(0x91, 48, bassvolume);
      break;
      case 86:
      midiwrite(0x91, 48, bassvolume);
      break;
      case 88:
      midiwrite(0x81, 48, 0x00); 
      midiwrite(0x91, 53, bassvolume);
      break;
      case 90:
      midiwrite(0x91, 53, bassvolume); 
      break;
      case 94:
      midiwrite(0x81, 53, 0x00); 
      midiwrite(0x91, 52, bassvolume);
      break;
      case 95:
      midiwrite(0x81, 52, 0x00); 
      break;
      case 96:
      midiwrite(0x91, 51, bassvolume);
      break;
      case 102:
      midiwrite(0x91, 58, bassvolume);
      break;
      case 103:
      midiwrite(0x81, 58, 0x00);
      midiwrite(0x91, 63, bassvolume);
      break;
      case 104:
      midiwrite(0x81, 63, 0x00); 
      midiwrite(0x91, 65, bassvolume);
      break;
      case 107:
      midiwrite(0x81, 65, 0x00); 
      midiwrite(0x91, 63, bassvolume);
      break;
      case 110:
      midiwrite(0x81, 63, 0x00); 
      midiwrite(0x91, 58, bassvolume);
      break;
      case 112:
      midiwrite(0x81, 58, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 115:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 63, bassvolume);
      break;
      case 118:
      midiwrite(0x81, 63, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 120:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 63, bassvolume);
      break;
    
    default :
      break;
    }    
}

void basssubseq5(long relativpos) 
{
  
  switch(relativpos)
    {
    
      case 0:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 4:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 6:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 8:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
      case 10:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
      case 12:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 14:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
       case 18:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
       case 20:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 22:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 24:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
       case 26:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
      case 28:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 30:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
       case 32:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 34:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 35:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 36:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 38:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
       case 39:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 40:
      midiwrite(0x91, 41, bassvolume);
      break;
      case 42:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
       case 44:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 46:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 50:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 51:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 52:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 54:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 55:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 56:
      midiwrite(0x91, 41, bassvolume);
      break;
      case 58:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
      case 60:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 62:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 64:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 66:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 67:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 68:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 70:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
       case 71:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 72:
      midiwrite(0x91, 41, bassvolume);
      break;
      case 74:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
       case 76:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 78:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 82:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 83:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 84:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 86:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 87:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 88:
      midiwrite(0x91, 41, bassvolume);
      break;
      case 90:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
      case 92:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 94:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 96:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 98:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 99:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 100:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 102:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
       case 103:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 104:
      midiwrite(0x91, 41, bassvolume);
      break;
      case 106:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 43, bassvolume);
      break;
       case 108:
      midiwrite(0x81, 43, 0x00); 
      midiwrite(0x91, 38, bassvolume);
      break;
      case 110:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 114:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 115:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 116:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 118:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 119:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 120:
      midiwrite(0x91, 36, bassvolume);
      break;
      case 122:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 48, bassvolume);
      break;
      case 123:
      midiwrite(0x81, 48, 0x00); 
      break;
      case 124:
      midiwrite(0x91, 47, bassvolume);
      break;
      case 126:
      midiwrite(0x81, 47, 0x00); 
      midiwrite(0x91, 47, bassvolume);
      break;
      case 127:
      midiwrite(0x81, 47, 0x00); 
      break;
    
          
    default :
      break;
    }    
}

void basssubseq6(long relativpos) 
{
  
  switch(relativpos)
    {
    
      case 0:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 4:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 6:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 8:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 10:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
      case 12:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 14:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
       case 18:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
       case 20:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 22:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 24:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
       case 26:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
      case 28:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 30:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
       case 32:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 34:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 35:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 36:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 38:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
       case 39:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 40:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 42:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
       case 44:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 46:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 50:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 51:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 52:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 54:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 55:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 56:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 58:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
      case 60:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 62:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 64:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 66:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 67:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 68:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 70:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
       case 71:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 72:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 74:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
       case 76:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 78:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 82:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 83:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 84:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 86:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 87:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 88:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 90:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
      case 92:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 94:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 96:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 98:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 99:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 100:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 102:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
       case 103:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 104:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 106:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 41, bassvolume);
      break;
       case 108:
      midiwrite(0x81, 41, 0x00); 
      midiwrite(0x91, 36, bassvolume);
      break;
      case 110:
      midiwrite(0x81, 36, 0x00); 
      midiwrite(0x91, 34, bassvolume);
      break;
      case 114:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 115:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 116:
      midiwrite(0x91, 34, bassvolume);
      break;
      case 118:
      midiwrite(0x81, 34, 0x00); 
      midiwrite(0x91, 46, bassvolume);
      break;
      case 119:
      midiwrite(0x81, 46, 0x00); 
      break;
      case 120:
      midiwrite(0x91, 39, bassvolume);
      break;
      case 122:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 51, bassvolume);
      break;
      case 123:
      midiwrite(0x81, 51, 0x00); 
      break;
      case 124:
      midiwrite(0x91, 38, bassvolume);
      break;
      case 126:
      midiwrite(0x81, 38, 0x00); 
      midiwrite(0x91, 50, bassvolume);
      break;
      case 127:
      midiwrite(0x81, 50, 0x00); 
      break;
    
          
    default :
      break;
    }    
}


void basssubseq7(long relativpos) 
{
  
  switch(relativpos)
    {
    
      case 0:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 4:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 6:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 8:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 42, bassvolume);
      break;
      case 10:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
      case 12:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 14:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
       case 18:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
       case 20:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 22:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 24:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 42, bassvolume);
      break;
       case 26:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
      case 28:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 30:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
       case 32:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 34:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 35:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 36:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 38:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
       case 39:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 40:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 42:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
       case 44:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 46:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 50:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 51:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 52:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 54:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 55:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 56:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 58:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
      case 60:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 62:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 64:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 66:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 67:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 68:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 70:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
       case 71:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 72:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 74:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
       case 76:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 78:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 82:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 83:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 84:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 86:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 87:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 88:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 90:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
      case 92:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 94:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 96:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 98:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 99:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 100:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 102:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
       case 103:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 104:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 106:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
       case 108:
      midiwrite(0x81, 44, 0x00); 
      midiwrite(0x91, 39, bassvolume);
      break;
      case 110:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
      case 114:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 115:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 116:
      midiwrite(0x91, 37, bassvolume);
      break;
      case 118:
      midiwrite(0x81, 37, 0x00); 
      midiwrite(0x91, 49, bassvolume);
      break;
      case 119:
      midiwrite(0x81, 49, 0x00); 
      break;
      case 120:
      midiwrite(0x91, 42, bassvolume);
      break;
      case 122:
      midiwrite(0x81, 42, 0x00); 
      midiwrite(0x91, 44, bassvolume);
      break;
      case 124:
      midiwrite(0x81, 44, 0x00);
      midiwrite(0x91, 39, bassvolume);
      break;
      case 126:
      midiwrite(0x81, 39, 0x00); 
      midiwrite(0x91, 37, bassvolume);
      break;
    
          
    default :
      break;
    }    
}


// also here it could be nice to use arrays instead of coding it by hand
// something like:

bool closedHihatNotes[] = {0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,1,1,0,1,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,1,1,0,1,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,0};
bool openHihatNotes[] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
bool cymbalNotes[] =      {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
bool snareDrumNotes[] =   {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1};
bool bassDrumNotes[] =    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
 
  void drumsubseq(long relativpos)
  {
    // play the notes
    if (closedHihatNotes[relativpos]) {
      midiwrite(0x99, _CLOSEDHIHAT_NOTE, drumsvolume);
    }
    if (snareDrumNotes[relativpos]) {
      midiwrite(0x99, _SNAREDRUM_NOTE, drumsvolume);
    }
    if (bassDrumNotes[relativpos]) {
      midiwrite(0x99, _BASSDRUM_NOTE, drumsvolume);
    }
    if (openHihatNotes[relativpos]) {
      midiwrite(0x99, _OPENHIHAT_NOTE, drumsvolume);
    }
    if (cymbalNotes[relativpos]) {
      midiwrite(0x99, _CYMBAL_NOTE, drumsvolume);
    }
  }


bool openHihatNotes2[] =   {0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0};  
bool pedalHihatNotes2[] =  {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};  
bool cymbalNotes2[] =      {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
bool snareDrumNotes2[] =   {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,0};
bool bassDrumNotes2[] =    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
 
  void drumsubseq2(long relativpos)
  {

    if (snareDrumNotes2[relativpos]) {
      midiwrite(0x99, _SNAREDRUM_NOTE, drumsvolume);
    }
    if (bassDrumNotes2[relativpos]) {
      midiwrite(0x99, _BASSDRUM_NOTE, drumsvolume);
    }
    if (openHihatNotes2[relativpos]) {
      midiwrite(0x99, _OPENHIHAT_NOTE, drumsvolume);
    }
    if (pedalHihatNotes2[relativpos]) {
      midiwrite(0x99, _PEDALHIHAT_NOTE, drumsvolume);
    }
    if (cymbalNotes2[relativpos]) {
      midiwrite(0x99, _CYMBAL_NOTE, drumsvolume);
    }
  }
  
  
//    // you could also go and try out mathematical formulas
//    if (sin(relativpos * 0.125 * PI) > 0.5) {
//      midiwrite(0x91, _BASSDRUM_NOTE, drumvolume);
//    }
// 
 


void synthsubseq(long relativpos) 
{
  
  switch(relativpos)
    {
      case 0:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 33, synthvolume);
      break;
      case 4:
      midiwrite(0x80, 33, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 6:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 33, synthvolume);
      break;
      case 8:
      midiwrite(0x80, 33, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 10:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 12:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 14:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 33, synthvolume);
      break;
       case 18:
      midiwrite(0x80, 33, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
       case 20:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 33, synthvolume);
      break;
      case 22:
      midiwrite(0x80, 33, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 24:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
       case 26:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 28:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 30:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 33, synthvolume);
      break;
       case 32:
      midiwrite(0x80, 33, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 34:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 36:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 42, synthvolume);
      break;
       case 38:
      midiwrite(0x80, 42, 0x00); 
      midiwrite(0x90, 43, synthvolume);
      break;
      case 42:
      midiwrite(0x80, 43, 0x00); 
      midiwrite(0x90, 42, synthvolume);
      break;
       case 44:
      midiwrite(0x80, 42, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 46:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 43, synthvolume);
      break;
      case 48:
      midiwrite(0x80, 43, 0x00); 
      midiwrite(0x90, 42, synthvolume);
      break;
      case 52:
      midiwrite(0x80, 42, 0x00); 
      midiwrite(0x90, 37, synthvolume);
      break;
      case 54:
      midiwrite(0x80, 37, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 56:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 60:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 62:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 64:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 41, synthvolume);
      break;
      case 66:
      midiwrite(0x80, 41, 0x00); 
      midiwrite(0x90, 43, synthvolume);
      break;
      case 68:
      midiwrite(0x80, 43, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 70:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 46, synthvolume);
      break;
      case 74:
      midiwrite(0x80, 46, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 76:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 43, synthvolume);
      break;
      case 78:
      midiwrite(0x80, 43, 0x00); 
      midiwrite(0x90, 46, synthvolume);
      break;
      case 80:
      midiwrite(0x80, 46, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 84:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 86:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 41, synthvolume);
      break;
      case 88:
      midiwrite(0x80, 41, 0x00); 
      midiwrite(0x90, 43, synthvolume);
      break;
      case 92:
      midiwrite(0x80, 43, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 94:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 96:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 41, synthvolume);
      break;
      case 98:
      midiwrite(0x80, 41, 0x00); 
      midiwrite(0x90, 36, synthvolume);
      break;
      case 102:
      midiwrite(0x80, 36, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 104:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 40, synthvolume);
      break;
      case 108:
      midiwrite(0x80, 40, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 112:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 116:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 120:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
      case 122:
      midiwrite(0x80, 35, 0x00); 
      midiwrite(0x90, 38, synthvolume);
      break;
      case 124:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 36, synthvolume);
      break;
      case 126:
      midiwrite(0x80, 36, 0x00); 
      midiwrite(0x90, 35, synthvolume);
      break;
   
    default :
      break;
    }    
}


void synthsubseq3(long relativpos) 
{
  
  switch(relativpos)
    {
      case 0:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 1:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);  
      break;
      case 2: 
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 5:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      break;
      case 6:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 7:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      break;
      case 8:
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 9:
      midiwrite(0x80, 55, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      break;
      case 10:
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 13:
      midiwrite(0x80, 55, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      break;
      case 14:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
       case 17:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 60, 0x00);  
      midiwrite(0x80, 65, 0x00);
      break;
      case 18:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
       case 19:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 60, 0x00);  
      midiwrite(0x80, 65, 0x00);
      break;
      case 20:
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 67, synthvolume);
      break;
      case 24:
      midiwrite(0x80, 59, 0x00);
      midiwrite(0x80, 64, 0x00); 
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 28:
      midiwrite(0x80, 60, 0x00);
      midiwrite(0x80, 64, 0x00); 
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 67, synthvolume);
      break;
      case 30:
      midiwrite(0x80, 59, 0x00);
      midiwrite(0x80, 64, 0x00); 
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 32:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      midiwrite(0x90, 53, synthvolume);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      break;
      case 36:
      midiwrite(0x80, 53, 0x00);
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 40:
      midiwrite(0x80, 55, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 48:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      midiwrite(0x90, 53, synthvolume);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      break;
      case 52:
      midiwrite(0x80, 53, 0x00);
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 67, synthvolume);
      break;
      case 56:
      midiwrite(0x80, 58, 0x00);
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00); 
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 64:
      midiwrite(0x80, 60, 0x00);
      midiwrite(0x80, 64, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 65:
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);  
      break;
      case 66: 
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 69:
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      break;
      case 70:
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      break;
      case 71:
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 65, 0x00);
      break;
      case 72:
      midiwrite(0x90, 67, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 73:
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      break;
      case 74:
      midiwrite(0x90, 67, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 77:
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      break;
      case 78:
      midiwrite(0x90, 67, synthvolume);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 64, synthvolume);
      break;
      case 79:
      midiwrite(0x80, 67, 0x00);
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 64, 0x00);
      break;
      case 80:
      midiwrite(0x90, 53, synthvolume);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      break;
      case 91:
      midiwrite(0x80, 53, 0x00);
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x80, 59, 0x00); 
      midiwrite(0x80, 62, 0x00);
      break;
      case 92:
      midiwrite(0x90, 53, synthvolume);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      break;
      case 96:
      midiwrite(0x80, 53, 0x00);
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x80, 59, 0x00); 
      midiwrite(0x80, 62, 0x00);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 112:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 61, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      break;
      case 114:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 115:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 61, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      break;
      case 116:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 117:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 61, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      break;
      case 118:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 119:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 61, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      break;
      case 120:
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      break;
      case 121:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 61, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 69, 0x00);
      break;
   
    default :
      break;
    }    
}

void synthsubseq4(long relativpos) 
{
  
  switch(relativpos)
    {
      case 80:
      midiwrite(0x90, 81, synthvolume);
      break;
      case 81:
      midiwrite(0x80, 81, 0x00);
      midiwrite(0x90, 77, synthvolume);
      break;
      case 82:
      midiwrite(0x80, 77, 0x00);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 83:
      midiwrite(0x80, 79, 0x00);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 84:
      midiwrite(0x80, 81, 0x00);
      break;
      case 85:
      midiwrite(0x90, 77, synthvolume);
      break;
      case 86:
      midiwrite(0x80, 77, 0x00);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 87:
      midiwrite(0x80, 79, 0x00);
      break;
      case 88:
      midiwrite(0x90, 81, synthvolume);
      break;
      case 89:
      midiwrite(0x80, 81, 0x00);
      break;
      case 96:
      midiwrite(0x90, 81, synthvolume);
      break;
      case 97:
      midiwrite(0x80, 81, 0x00);
      midiwrite(0x90, 77, synthvolume);
      break;
      case 98:
      midiwrite(0x80, 77, 0x00);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 99:
      midiwrite(0x80, 79, 0x00);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 100:
      midiwrite(0x80, 81, 0x00);
      break;
      case 101:
      midiwrite(0x90, 77, synthvolume);
      break;
      case 102:
      midiwrite(0x80, 77, 0x00);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 103:
      midiwrite(0x80, 79, 0x00);
      break;
      case 104:
      midiwrite(0x90, 81, synthvolume);
      break;
      case 105:
      midiwrite(0x80, 81, 0x00);
      break;
      case 114:
      midiwrite(0x90, 73, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 115:
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x80, 77, 0x00); 
      midiwrite(0x80, 81, 0x00); 
      break;
      case 116:
      midiwrite(0x90, 73, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 117:
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x80, 77, 0x00); 
      midiwrite(0x80, 81, 0x00);
      break;
      case 118:
      midiwrite(0x90, 73, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 119:
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x80, 77, 0x00); 
      midiwrite(0x80, 81, 0x00);
      break;
      case 120:
      midiwrite(0x90, 73, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 81, synthvolume);
      break;
      case 121:
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x80, 77, 0x00); 
      midiwrite(0x80, 81, 0x00);
      break;


default :
      break;
    }    
}



void synthsubseq5(long relativpos) 
{
  
  switch(relativpos)
    {
      case 0:
      midiwrite(0x80, 59, 0x00);
      midiwrite(0x80, 69, 0x00); 
      midiwrite(0x80, 73, 0x00); 
      midiwrite(0x80, 77, 0x00);
      midiwrite(0x80, 81, 0x00); 
      midiwrite(0x90, 61, synthvolume);
      midiwrite(0x90, 63, synthvolume);
      midiwrite(0x90, 66, synthvolume);
      midiwrite(0x90, 70, synthvolume);
      break;
      case 8:
      midiwrite(0x80, 61, 0x00);
      midiwrite(0x80, 63, 0x00); 
      midiwrite(0x80, 66, 0x00); 
      midiwrite(0x80, 70, 0x00);
      midiwrite(0x90, 57, synthvolume);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 68, synthvolume);
      break;
      case 16:
      midiwrite(0x80, 57, 0x00);
      midiwrite(0x80, 60, 0x00); 
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 68, 0x00);
      midiwrite(0x90, 56, synthvolume);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 63, synthvolume);
      midiwrite(0x90, 66, synthvolume);
      midiwrite(0x90, 70, synthvolume);
      break;
      case 24:
      midiwrite(0x80, 56, 0x00);
      midiwrite(0x80, 59, 0x00); 
      midiwrite(0x80, 63, 0x00);
      midiwrite(0x80, 66, 0x00);
      midiwrite(0x80, 70, 0x00);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 70, synthvolume);
      break;
      case 32:
      midiwrite(0x80, 55, 0x00);
      midiwrite(0x80, 62, 0x00);
      midiwrite(0x80, 65, 0x00);
      midiwrite(0x80, 70, 0x00);
      midiwrite(0x90, 64, synthvolume);
      midiwrite(0x90, 66, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 73, synthvolume);
      break;
      case 40:
      midiwrite(0x80, 64, 0x00);
      midiwrite(0x80, 66, 0x00); 
      midiwrite(0x80, 69, 0x00); 
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 63, synthvolume);
      midiwrite(0x90, 68, synthvolume);
      midiwrite(0x90, 71, synthvolume);
      break;
      case 48:
      midiwrite(0x80, 60, 0x00);
      midiwrite(0x80, 63, 0x00); 
      midiwrite(0x80, 69, 0x00); 
      midiwrite(0x80, 71, 0x00);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 66, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 73, synthvolume);
      break;
      case 56:
      midiwrite(0x80, 59, 0x00);
      midiwrite(0x80, 62, 0x00); 
      midiwrite(0x80, 66, 0x00); 
      midiwrite(0x80, 69, 0x00);
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x90, 58, synthvolume);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 68, synthvolume);
      midiwrite(0x90, 73, synthvolume);
      break;
      case 64:
      midiwrite(0x80, 58, 0x00);
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x80, 68, 0x00); 
      midiwrite(0x80, 73, 0x00);
      midiwrite(0x90, 55, synthvolume);
      midiwrite(0x90, 67, synthvolume);
      midiwrite(0x90, 71, synthvolume);
      midiwrite(0x90, 72, synthvolume);
      midiwrite(0x90, 76, synthvolume);
      break;
      case 72:
      midiwrite(0x80, 55, 0x00);
      midiwrite(0x80, 67, 0x00); 
      midiwrite(0x80, 71, 0x00); 
      midiwrite(0x80, 72, 0x00);
      midiwrite(0x80, 76, 0x00);
      midiwrite(0x90, 62, synthvolume);
      midiwrite(0x90, 66, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 72, synthvolume);
      midiwrite(0x90, 76, synthvolume);
      break;
      case 80:
      midiwrite(0x80, 62, 0x00);
      midiwrite(0x80, 66, 0x00); 
      midiwrite(0x80, 69, 0x00); 
      midiwrite(0x80, 72, 0x00);
      midiwrite(0x80, 76, 0x00);
      midiwrite(0x90, 60, synthvolume);
      midiwrite(0x90, 70, synthvolume);
      midiwrite(0x90, 74, synthvolume);
      midiwrite(0x90, 75, synthvolume);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 88:
      midiwrite(0x80, 60, 0x00);
      midiwrite(0x80, 70, 0x00); 
      midiwrite(0x80, 74, 0x00); 
      midiwrite(0x80, 75, 0x00);
      midiwrite(0x80, 79, 0x00);
      midiwrite(0x90, 65, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 72, synthvolume);
      midiwrite(0x90, 75, synthvolume);
      midiwrite(0x90, 79, synthvolume);
      break;
      case 96:
      midiwrite(0x80, 65, 0x00);
      midiwrite(0x80, 69, 0x00); 
      midiwrite(0x80, 72, 0x00); 
      midiwrite(0x80, 75, 0x00);
      midiwrite(0x80, 79, 0x00);
      midiwrite(0x90, 63, synthvolume);
      midiwrite(0x90, 72, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 78, synthvolume);
      midiwrite(0x90, 82, synthvolume);
      break;
      case 112:
      midiwrite(0x80, 63, 0x00);
      midiwrite(0x80, 72, 0x00); 
      midiwrite(0x80, 77, 0x00); 
      midiwrite(0x80, 78, 0x00);
      midiwrite(0x80, 82, 0x00);
      midiwrite(0x90, 59, synthvolume);
      midiwrite(0x90, 69, synthvolume);
      midiwrite(0x90, 73, synthvolume);
      midiwrite(0x90, 77, synthvolume);
      midiwrite(0x90, 81, synthvolume);
      break;
   
    default :
      break;
    }    
}

void synthsubseq6(long relativpos) 
{
  
  switch(relativpos)
    {
      case 0:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 70, synthvolume);
      break;
      case 5:
      midiwrite(0x80, 70, 0x00); 
      midiwrite(0x90, 65, synthvolume);
      break;
      case 6:
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x90, 66, synthvolume);
      break;
      case 7:
      midiwrite(0x80, 66, 0x00); 
      break;
      case 8:
      midiwrite(0x90, 68, synthvolume);
      break;
      case 12:
      midiwrite(0x80, 68, 0x00); 
      break;
      case 16:
      midiwrite(0x90, 70, synthvolume);
      break;
       case 21:
      midiwrite(0x80, 70, 0x00); 
      midiwrite(0x90, 65, synthvolume);
      break;
       case 22:
      midiwrite(0x80, 65, 0x00); 
      midiwrite(0x90, 66, synthvolume);
      break;
      case 23:
      midiwrite(0x80, 66, 0x00); 
      break;
      case 24:
      midiwrite(0x90, 70, synthvolume);
      break;
       case 28:
      midiwrite(0x80, 70, 0x00); 
      break;
      case 30:
      midiwrite(0x90, 72, synthvolume);
      break;
      case 32:
      midiwrite(0x80, 72, 0x00); 
      midiwrite(0x90, 73, synthvolume);
      break;
      case 37:
      midiwrite(0x80, 73, 0x00); 
      midiwrite(0x90, 68, synthvolume);
      break;
       case 38:
      midiwrite(0x80, 68, 0x00); 
      midiwrite(0x90, 69, synthvolume);
      break;
      case 39:
      midiwrite(0x80, 69, 0x00); 
      break;
      case 40:
      midiwrite(0x90, 71, synthvolume);
      break;
       case 44:
      midiwrite(0x80, 71, 0x00); 
      break;
      case 48:
      midiwrite(0x90, 73, synthvolume);
      break;
      case 53:
      midiwrite(0x80, 73, 0x00); 
      midiwrite(0x90, 68, synthvolume);
      break;
      case 54:
      midiwrite(0x80, 68, 0x00); 
      midiwrite(0x90, 69, synthvolume);
      break;
      case 55:
      midiwrite(0x80, 69, 0x00); 
      break;
      case 56:
      midiwrite(0x90, 73, synthvolume);
      break;
      case 62:
      midiwrite(0x80, 73, 0x00); 
      midiwrite(0x90, 75, synthvolume);
      break;
      case 64:
      midiwrite(0x80, 75, 0x00); 
      midiwrite(0x90, 76, synthvolume);
      break;
      case 69:
      midiwrite(0x80, 76, 0x00); 
      midiwrite(0x90, 71, synthvolume);
      break;
      case 70:
      midiwrite(0x80, 71, 0x00); 
      midiwrite(0x90, 72, synthvolume);
      break;
      case 71:
      midiwrite(0x80, 72, 0x00); 
      break;
      case 72:
      midiwrite(0x90, 76, synthvolume);
      break;
      case 76:
      midiwrite(0x80, 76, 0x00); 
      break;
      case 78:
      midiwrite(0x90, 78, synthvolume);
      break;
      case 80:
      midiwrite(0x80, 78, 0x00); 
      midiwrite(0x90, 79, synthvolume);
      break;
      case 85:
      midiwrite(0x80, 79, 0x00); 
      midiwrite(0x90, 74, synthvolume);
      break;
      case 86:
      midiwrite(0x80, 74, 0x00); 
      midiwrite(0x90, 75, synthvolume);
      break;
      case 87:
      midiwrite(0x80, 75, 0x00); 
      break;
      case 88:
      midiwrite(0x90, 79, synthvolume);
      break;
      case 92:
      midiwrite(0x80, 79, 0x00); 
      break;
      case 94:
      midiwrite(0x90, 81, synthvolume);
      break;
      case 96:
      midiwrite(0x80, 81, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 97:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 98:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 99:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 100:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 101:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 102:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 103:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 104:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 105:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 106:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 107:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 108:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 109:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 110:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 111:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 112:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 113:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 114:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 115:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 116:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 117:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 118:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 119:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 120:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 121:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 123:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 124:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 125:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
      case 126:
      midiwrite(0x80, 82, 0x00); 
      midiwrite(0x90, 84, synthvolume);
      break;
      case 127:
      midiwrite(0x80, 84, 0x00); 
      midiwrite(0x90, 82, synthvolume);
      break;
   
    default :
      break;
    }    
}


void synthsubseq7(long relativpos) 
{
  
  switch(relativpos)
    {
     case 0:
      midiwrite(0x80, 38, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 4:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 6:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 8:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 10:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 12:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 14:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
       case 18:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
       case 20:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
      case 22:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 24:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
       case 26:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 28:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 30:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 45, synthvolume);
      break;
       case 32:
      midiwrite(0x80, 45, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 34:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 36:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 54, synthvolume);
      break;
       case 38:
      midiwrite(0x80, 54, 0x00); 
      midiwrite(0x90, 55, synthvolume);
      break;
      case 42:
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x90, 54, synthvolume);
      break;
       case 44:
      midiwrite(0x80, 54, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 46:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 55, synthvolume);
      break;
      case 48:
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x90, 54, synthvolume);
      break;
      case 52:
      midiwrite(0x80, 54, 0x00); 
      midiwrite(0x90, 49, synthvolume);
      break;
      case 54:
      midiwrite(0x80, 49, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 56:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 60:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 62:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 64:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 53, synthvolume);
      break;
      case 66:
      midiwrite(0x80, 53, 0x00); 
      midiwrite(0x90, 55, synthvolume);
      break;
      case 68:
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x90, 57, synthvolume);
      break;
      case 70:
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x90, 58, synthvolume);
      break;
      case 74:
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x90, 57, synthvolume);
      break;
      case 76:
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x90, 55, synthvolume);
      break;
      case 78:
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x90, 58, synthvolume);
      break;
      case 80:
      midiwrite(0x80, 58, 0x00); 
      midiwrite(0x90, 57, synthvolume);
      break;
      case 84:
      midiwrite(0x80, 57, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 86:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 53, synthvolume);
      break;
      case 88:
      midiwrite(0x80, 53, 0x00); 
      midiwrite(0x90, 55, synthvolume);
      break;
      case 92:
      midiwrite(0x80, 55, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 94:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 96:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 53, synthvolume);
      break;
      case 98:
      midiwrite(0x80, 53, 0x00); 
      midiwrite(0x90, 48, synthvolume);
      break;
      case 102:
      midiwrite(0x80, 48, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 104:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 52, synthvolume);
      break;
      case 108:
      midiwrite(0x80, 52, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 112:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 116:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 120:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
      case 122:
      midiwrite(0x80, 47, 0x00); 
      midiwrite(0x90, 50, synthvolume);
      break;
      case 124:
      midiwrite(0x80, 50, 0x00); 
      midiwrite(0x90, 48, synthvolume);
      break;
      case 126:
      midiwrite(0x80, 48, 0x00); 
      midiwrite(0x90, 47, synthvolume);
      break;
   
    default :
      break;
    }    
}


 void synthseq(long seqpos) 
{ 
  char relativpos;
  char seq_poscnt_mod2;
  relativpos =seqpos%128;
  seq_poscnt_mod2 = seqpos%2;
  if(enablesynthline)
  {
  switch(relativpos)
    {
   
//    case 14:
//        midiwrite(0x80, 53, 0x00); 
//        midiwrite(0x80, 57, 0x00); 
//       midiwrite(0x90, 50, synthvolume); 
//       midiwrite(0x90, 55, synthvolume); 
//       midiwrite(0x90, 59, synthvolume);    
//      break;
//    case 30 :
//      midiwrite(0x80, 50, 0x00); 
//      midiwrite(0x80, 55, 0x00); 
//      midiwrite(0x80, 59, 0x00); 
//       midiwrite(0x90, 52, synthvolume); 
//       midiwrite(0x90, 57, synthvolume); 
//       midiwrite(0x90, 60, synthvolume);     
//      break;
//    case 46:
//      midiwrite(0x80, 52, 0x00); 
//       midiwrite(0x90, 53, synthvolume); 
//       midiwrite(0x90, 57, synthvolume); 
//       midiwrite(0x90, 60, synthvolume);
//      break;  
//      case 62 :
//       midiwrite(0x80, 60, 0x00);  
//       midiwrite(0x90, 50, synthvolume); 
//       midiwrite(0x90, 53, synthvolume); 
//       midiwrite(0x90, 57, synthvolume);
//      break;   

    
   
    default :
      break;
    }    

if(enablebassline)
    {
      basssubseq(relativpos);
    }

    if(enablebassline2)
    {
      basssubseq2(relativpos);
    }

    if(enablebassline4)
    {
      basssubseq4(relativpos);
    }

    if(enablebassline5)
    {
      basssubseq5(relativpos);
    }

    if(enablebassline6)
    {
      basssubseq6(relativpos);
    }

    if(enablebassline7)
    {
      basssubseq7(relativpos);
    }

    if(enablesynthline2)
    {
      synthsubseq(relativpos);
    }

    if(enablesynthline3)
    {
      synthsubseq3(relativpos);
    }

    if(enablesynthline4)
    {
      synthsubseq4(relativpos);
    }
    
    if(enablesynthline5)
    {
      synthsubseq5(relativpos);
    }

    if(enablesynthline6)
    {
      synthsubseq6(relativpos);
    }

    if(enablesynthline7)
    {
      synthsubseq7(relativpos);
    }

    if(enablebassline3)
    {
      basssubseq3(relativpos);
    }
    
   if(enabledrumming)
   {
      drumsubseq(relativpos);     
   }

  if(enabledrumming2)
   {
      drumsubseq2(relativpos);     
   }
   
  } 
 };


void timercallback()
{
  lasttickstamp = millis();
  Timer1.setPeriod(tempodelay * 1000);     
  tickflag = 1;
}

void setupvoices(void)
{
    midiprogchange(0xC0,channel1_prog);
    midiprogchange(0xC1,channel2_prog);
}
void SomeButtonPressHandler(Button& butt)
{  
 if(enablesynthline)
  {
    if(butt == button3)
    {
      enablebassline = enablebassline?0:1;
      enablebassline2 = enablebassline2?0:1;
      enabledrumming = enabledrumming?0:1;
    };

  }
  else
  {
    
    if(butt == button3)
    {  
      midiwrite(0x99, _SNAREDRUM_NOTE, 0x65);   
    }
    if(butt == button4)
    {  
      midiwrite(0x99, _BASSDRUM_NOTE, 0x65);   
    }
  }
}
void stopallnotes(byte selection = 0)
{  

if (selection == 0 || selection ==2)
{

  for (int i = 33; i <= 84; i++){
      midiwrite(0x90, i, 0x00);
  }
}

if (selection == 0 || selection ==1)
{

    for (int i = 33; i <= 84; i++){
      midiwrite(0x91, i, 0x00);
  }
  
     
}
}



void SomeButtonReleaseHandler(Button& butt)
{
  if(enablesynthline)
  {
    //..
  } 
  else
  {
  
  }
  
  
}

void PotControl(int potnr, int ctrlVal)
{
  if(potnr == 1)
  {
     //Serial.println(ctrlVal);
      midisetup_sam2195_nrpn_send(resonantchannel,0x01,0x20,(byte)ctrlVal); 
      midiwrite(0xB0, 0x01,(byte) ctrlVal);  // modulation wheel
      
       
  }
  if(potnr == 2)
  {
     midisetup_sam2195_nrpn_send(resonantchannel,0x01,0x21,(byte)ctrlVal);  // set resonance for channel resonantchannel
//    drumvolume = (byte)ctrlVal;
 }
}


void ADC_handle(void)
{
  static int prevsensorVal1 = -1000;  
  static int prevsensorVal2 = -1000;
  if(pot1setup)
  {
    int sensorValue = analogRead(A4);
    int ctrlValue;
    if(abs(sensorValue-prevsensorVal1)>3)  // filter noise from adc input
    {
//        Serial.print(sensorValue);
        prevsensorVal1 = sensorValue;
        ctrlValue = sensorValue/8;
        PotControl(1,ctrlValue);
    }
  }
  if(pot2setup)
  {
    int sensorValue = analogRead(A3);
    int ctrlValue;
    if(abs(sensorValue-prevsensorVal2)>3)  // filter noise from adc input
    {
//        Serial.print(" - ");
//        Serial.println(sensorValue);
        prevsensorVal2 = sensorValue;
        ctrlValue = sensorValue/8;
        PotControl(2,ctrlValue);
    }
  }
}


void SomeButtonHoldHandler(Button& butt)
{
  if(butt==button4)
  {
        stopsynth(); 
  }
}

/*
 * ideally also here you would try to work rather with an array of booleans than hardcoded booleans.
 * 
 * since your key is a char, and chars are also just numbers, you could even do something like
 * bool keyPressStatus[256];
 * 
 * // use a char to access the array
 * keyPressStatus['a'] = true;
 * keyPressStatus['b'] = false;
 * keyPressStatus[' '] = false;
 * 
 * if (iskeyrelease) {
 *   keyPressStatus[key] = false;
 * } else {
 *   if (!keyPressStatus[key]) {
 *     // things that should happen only once when a key is pressed
 *     keyPressStatus[key] = true;
 *     
 *     if (key == '*') {
 *       // handle special keys
 *     } else if (key == 'b') {
 *       // handle specific letters
 *     } else if (key >= 'a' && key <= 'Z') {
 *       // handle all other letters
 *     } else if (key >= '0' && key <= '9') {
 *       // handle all other numbers
 *     } 
 *   } else {
 *     // things that should happen continuously when a key is pressed
 *   }
 * }
 * 
 * and so on.
 * 
 * BTW, all this code here is C++
 * Meaning, you can test out little snippets in a sandbox and check if your assumptions work:
 * http://www.cpp.sh/
 * you can write your code in the "main()" function of the sandbox, and try things out
 * 
 * for example, you can check that you can indeed use "key" to access an array like keyPressStatus[key]:
 * // Example program
#include <iostream>
int main()
{
  char key = '0';
  // which number does the key have?
  std::cout << "number value of '" << key << "' is " << int(key) <<  " \n";
  
  bool keyPressStatus[256];
  
  keyPressStatus['a'] = false;
  keyPressStatus[key] = true;
  
  if (keyPressStatus['a']) {
      std::cout << "keyPressStatus['a'] is true!\n";
  }
  
  // using the number or the key as an index in the array should be equivalent,
  // so all the following statements should be true
  if (keyPressStatus[48]) {
      std::cout << "keyPressStatus[48] is true!\n";
  }
  
  if (keyPressStatus[key]) {
      std::cout << "keyPressStatus[key] is true!\n";
  }
  
  if (keyPressStatus['0']) {
      std::cout << "keyPressStatus['0'] is true!\n";
  }
  // except this one, because here we use a random empty index
  // to make sure that our list is initialized with "false" for each value
  if (!keyPressStatus[40]) {
      std::cout << "keyPressStatus[40] is false!\n";
  }
}
 * 
 */
// 

void KeyboardHandler(char key,bool iskeyrelease)
{
  static bool space_pressed = false;  
  static bool enter_pressed = false;  
  static bool zero_pressed = false;  
  static bool nine_pressed = false; 
  static bool six_pressed = false; 
  static bool seven_pressed = false; 
  static bool c_pressed = false;  
  static bool v_pressed = false;  
  static bool b_pressed = false;
  static bool a_pressed = false; 
  static bool s_pressed = false;
  static bool d_pressed = false;
  static bool f_pressed = false;
  static bool g_pressed = false;
  static bool h_pressed = false;
  static bool j_pressed = false;
  static bool k_pressed = false;
  static bool l_pressed = false; 
  static bool o_pressed = false;
  static bool p_pressed = false;
  static bool i_pressed = false;
  static bool u_pressed = false;
  static bool y_pressed = false;
  static bool t_pressed = false;
  static bool r_pressed = false;
  static bool e_pressed = false;
  static bool w_pressed = false;
  static bool q_pressed = false;
  static bool z_pressed = false;
  static bool x_pressed = false;
  static bool n_pressed = false;
  static bool m_pressed = false;
  byte drumstatuscode;
  byte synthstatuscode;
  byte volumecode;

  drumstatuscode = iskeyrelease ? _DRUMRELEASE_STATUSCODE : _DRUMPRESS_STATUSCODE;
  synthstatuscode = iskeyrelease ? _SYNTHRELEASE_STATUSCODE : _SYNTHPRESS_STATUSCODE;
  volumecode = iskeyrelease ? 0x00 : leadvolume;
  if((key==PS2_PAGEUP)|| (key==PS2_PAGEDOWN) || (key=='+')|| (key=='-'))
  {
    if(!iskeyrelease) 
     {
        if(key=='+') 
        {
          channel2_prog++;
          if(channel2_prog > 0x7F)
          {
            channel2_prog = 0x00; 
          }       
        } 
       if(key=='-') 
        {
          channel2_prog--;
          if(channel2_prog > 0x7F)
          {
            channel2_prog = 0x7F; 
          }       
        } 
       if(key==PS2_PAGEUP) 
        {
          channel1_prog++;
          if(channel1_prog > 0x7F)
          {
            channel1_prog = 0x00; 
          }       
        } 
       if(key==PS2_PAGEDOWN)
        {
          channel1_prog--;
          if(channel1_prog > 0x7F)
          {
            channel1_prog = 0x7F; 
          }       
        } 

       setupvoices();
     }
  }
  if(key==' ') 
   {
     if(iskeyrelease) 
     {     
     stopallnotes(1);
     stopallnotes(2);     
     }
     return;
   }
  if(key==PS2_ENTER) 
   {
     if(!enter_pressed  || iskeyrelease) 
     { 
      midiwrite(drumstatuscode,_CLOSEDHIHAT_NOTE, volumecode); 
     }
     enter_pressed = !iskeyrelease;
     return;
   }
  if(key=='0') 
   {
     if(!zero_pressed || iskeyrelease) 
     { 
      midiwrite(drumstatuscode,  _SNAREDRUM_NOTE, volumecode);      
     }
     zero_pressed = !iskeyrelease;
     return;
   }

   if(key=='9') 
   {
     if(!nine_pressed || iskeyrelease) 
     { 
      midiwrite(drumstatuscode,  _CYMBAL_NOTE, volumecode);      
     }
     nine_pressed = !iskeyrelease;
     return;
   }

   if(key=='a') 
   {
     if(!a_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  60, volumecode);      
     }
     a_pressed = !iskeyrelease;
     return;
   }

   if(key=='s') 
   {
     if(!s_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  62, volumecode);      
     }
     s_pressed = !iskeyrelease;
     return;
   }

   if(key=='d') 
   {
     if(!d_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  64, volumecode);      
     }
     d_pressed = !iskeyrelease;
     return;
   }
   if(key=='f') 
   {
     if(!f_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  65, volumecode);      
     }
     f_pressed = !iskeyrelease;
     return;
   }
   if(key=='g') 
   {
     if(!g_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  67, volumecode);      
     }
     g_pressed = !iskeyrelease;
     return;
   }

   if(key=='h') 
   {
     if(!h_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  69, volumecode);      
     }
     h_pressed = !iskeyrelease;
     return;
   }
   if(key=='j') 
   {
     if(!j_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  70, volumecode);      
     }
     j_pressed = !iskeyrelease;
     return;
   }
   if(key=='k') 
   {
     if(!k_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  72, volumecode);      
     }
     k_pressed = !iskeyrelease;
     return;
   }
   if(key=='l') 
   {
     if(!l_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  74, volumecode);      
     }
     l_pressed = !iskeyrelease;
     return;
   }
   if(key=='o') 
   {
     if(!o_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  76, volumecode);      
     }
     o_pressed = !iskeyrelease;
     return;
   }
   if(key=='p') 
   {
     if(!p_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  77, volumecode);      
     }
     p_pressed = !iskeyrelease;
     return;
   }
   if(key=='i') 
   {
     if(!i_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  74, volumecode);      
     }
     i_pressed = !iskeyrelease;
     return;
   }
   if(key=='u') 
   {
     if(!u_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  72, volumecode);      
     }
     u_pressed = !iskeyrelease;
     return;
   }
   if(key=='y') 
   {
     if(!y_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  60, volumecode);      
     }
     y_pressed = !iskeyrelease;
     return;
   }
   
   if(key=='t') 
   {
     if(!t_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  59, volumecode);      
     }
     t_pressed = !iskeyrelease;
     return;
   }
   if(key=='r') 
   {
     if(!r_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  57, volumecode);      
     }
     r_pressed = !iskeyrelease;
     return;
   }
   if(key=='e') 
   {
     if(!e_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  55, volumecode);      
     }
     e_pressed = !iskeyrelease;
     return;
   }
   if(key=='w') 
   {
     if(!w_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  52, volumecode);      
     }
     w_pressed = !iskeyrelease;
     return;
   }
   if(key=='q') 
   {
     if(!q_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  50, volumecode);      
     }
     q_pressed = !iskeyrelease;
     return;
   }
   if(key=='z') 
   {
     if(!z_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  76, volumecode);      
     }
     z_pressed = !iskeyrelease;
     return;
   }
   if(key=='x') 
   {
     if(!x_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  77, volumecode);      
     }
     x_pressed = !iskeyrelease;
     return;
   }
   if(key=='c') 
   {
     if(!c_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  79, volumecode);      
     }
     c_pressed = !iskeyrelease;
     return;
   }
   if(key=='v') 
   {
     if(!v_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  81, volumecode);      
     }
     v_pressed = !iskeyrelease;
     return;
   }
   if(key=='b') 
   {
     if(!b_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  83, volumecode);      
     }
     b_pressed = !iskeyrelease;
     return;
   }
   if(key=='n') 
   {
     if(!n_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  84, volumecode);      
     }
     n_pressed = !iskeyrelease;
     return;
   }
   if(key=='m') 
   {
     if(!m_pressed || iskeyrelease) 
     { 
      midiwrite(synthstatuscode,  33, volumecode);
      midiwrite(synthstatuscode,  45, volumecode);      
     }
     m_pressed = !iskeyrelease;
     return;
   }
   if(key=='1') 
   {
     if(!iskeyrelease) 
     {        
       buttonState = (buttonState==0) ? 1 : 0;        
     }
     return;
   }
   
  
   if(key=='2') 
   {
     if(!iskeyrelease) 
     { 
        enablebassline = enablebassline?0:1;
        if(enablebassline) { enablebassline4 = 0; stopallnotes(1);};

        enablesynthline3 = enablesynthline3?0:1;
        if(enablesynthline3) { enablesynthline5 = 0; enablesynthline6 = 0; stopallnotes(2);};

        enablesynthline4 = enablesynthline4?0:1;
        if(!enablesynthline4) {stopallnotes(2);};
     }
     return;
   }
   
   if(key=='3') 
   {
     if(!iskeyrelease) 
     {        
       enabledrumming = (enabledrumming==0) ? 1 : 0;
       if(enabledrumming) { enablebassline2 = 0; enablesynthline2 = 0; enablesynthline7 = 0; enabledrumming2 = 0; };        
     }
     return;
   }

if(key=='4') 
   {
     if(!iskeyrelease) 
     { 
        enablesynthline = enablesynthline?0:1;
        if(!enablesynthline) {stopallnotes(2);};
     }
     return;
   }

   if(key=='5') 
   {
     if(!iskeyrelease) 
     { 
        enablebassline2 = enablebassline2?0:1;
        if(!enablebassline2) {stopallnotes(1);};

        enablesynthline2 = enablesynthline2?0:1;
        if(!enablesynthline2) {stopallnotes(2);};

        enablesynthline7 = enablesynthline7?0:1;
        if(!enablesynthline7) {stopallnotes(2);};
     }
     return;
   }

   if(key=='6') 
   {
     if(!iskeyrelease) 
     { 
        enablesynthline5 = enablesynthline5?0:1;
        if(enablesynthline5) { enablesynthline3 = 0; enablesynthline4 = 0; stopallnotes(2);};

        enablesynthline6 = enablesynthline6?0:1;
        if(!enablesynthline6) {stopallnotes(2);};

        enablebassline4 = enablebassline4?0:1;
        if(enablebassline4) { enablebassline = 0; stopallnotes(1);};
     }
     return;
   }

   if(key=='7') 
   {
     if(!iskeyrelease) 
     { 
        enabledrumming2 = (enabledrumming2==0) ? 1 : 0; 
        if(enabledrumming2) { enabledrumming = 0; };
     }
     return;
   }

   if(key==',') 
   {
     if(!iskeyrelease) 
     { 
        enablebassline6 = enablebassline6?0:1;
        if(enablebassline6) { enablebassline5 = 0; stopallnotes(1);};

     }
     return;
   }

   if(key=='.') 
   {
     if(!iskeyrelease) 
     { 
        enablebassline5 = enablebassline5?0:1;
        if(enablebassline5) { enablebassline7 = 0; stopallnotes(1);};

     }
     return;
   }

   if(key=='/') 
   {
     if(!iskeyrelease) 
     { 
        enablebassline7 = enablebassline7?0:1;
        if(enablebassline7) { enablebassline6 = 0; stopallnotes(1);};

     }
     return;
   }
  
   
}

void setup() {
  Serial.begin(31250);
  analogReference(DEFAULT); 
  
  pinMode(dreamblaster_enable_pin ,OUTPUT);   // enable the dreamblaster module by pulling high /reset
  digitalWrite(dreamblaster_enable_pin, HIGH);
  delay(300);  // allow 500ms for the dreamblaster to boot
  midisetup_sam2195_gsreset();
  delay(500);  // allow 250 for gs reset
  
  keyboard.begin(DataPin, IRQpin); // ps2 keyboard
  

  midisetup_sam2195_sysexcfg_send(0x02,0x00,0x01,0x50);  // set mod wheel cutoff
  midisetup_sam2195_sysexcfg_send(0x02,0x00,0x04,0x00);  // disable mod wheel pitch variation
  midisetup_sam2195_sysexcfg_send(0x02,0x00,0x05,0x7F);  // set mod wheel tvf depth

  midisetup_sam2195_nrpn_send(resonantchannel,0x01,0x21,(byte)0x7F);  // set resonance for channel resonantchannel
  Timer1.attachInterrupt(timercallback);
  Timer1.initialize(tempodelay*1000);   
  
  button3.pressHandler(SomeButtonPressHandler);
  button3.releaseHandler(SomeButtonReleaseHandler);  
  button4.pressHandler(SomeButtonPressHandler);
  button4.releaseHandler(SomeButtonReleaseHandler);
  button4.holdHandler(SomeButtonHoldHandler,1000);
  setupvoices();
}

void stopsynth(){
  enablesynthline = 0; 
  prevtimestamp = 0;
  curtimestamp = 0;
  seq_poscnt = 0;
  stopallnotes();
}


void loop() {
  long timesincelastbeat;
  long timetonextbeat;  
  
  ADC_handle();
  button3.isPressed(); // trigger handling for button
  button4.isPressed(); // trigger handling for button
  
  if (keyboard.availablerel()) {
      // read the next key
      bool iskeyrelease = false;
      char c = keyboard.readrel(iskeyrelease);      
      KeyboardHandler(c,iskeyrelease);
  }
  
  if(tickflag)
  {
    // sequencing time tick
    synthseq(seq_poscnt);
    seq_poscnt++;
    tickflag = 0;

  }
  
  if(prev_buttonState!=buttonState)
  {
    prev_buttonState = buttonState;
    if(buttonState == 1)
    {
      prevtimestamp =  curtimestamp;
      curtimestamp = millis();
      if(prevtimestamp)
      {           
        calctempodelay = curtimestamp - prevtimestamp;
        if(calctempodelay > 100  && calctempodelay < 3000)  // if tempo is within range, calc tempo and enable drum
        {
          if(enablesynthline == 0)
          {
            seq_poscnt = 0;
            enablesynthline = 1;
          }
          tempodelay  = calctempodelay/wheelfactor;
          timesincelastbeat = curtimestamp - lasttickstamp;
          if(timesincelastbeat > tempodelay)  // is the beat is going faster, tick immediately (within 10ms)
          {
            timetonextbeat = 10; 
          }
          else
          {
            timetonextbeat = tempodelay -timesincelastbeat; // calculate remaining period for next tick
            if (timetonextbeat < 10)
            {
              timetonextbeat = 10;
            }
          }
          Timer1.setPeriod(timetonextbeat * 1000);  // calculate period until next beat            
        }
      }
    }
  } 
  else
  {
    if(enablesynthline)
    {
      if(curtimestamp)
      {
        if( (millis() - curtimestamp  )> 5000)  // automatically stop if no sensor change for longer than 5s
        {
          if(bicyclesetup)
            {
               stopsynth(); 
            }
        }
      }
    }
  }

}
