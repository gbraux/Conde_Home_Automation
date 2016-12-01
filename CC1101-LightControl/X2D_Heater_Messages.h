#include "Arduino.h"
#include "ccpacket.h"

// The following values are taken from my own X2D installation and will not work as is, 
// unless you associates your receivers with my personnal values (but in this case your Deltia Emitter will not command your heaters until you associates them to Deltia Emitter again)
// To capture your own Deltia Messages use a compatible to SDR (RTL_SDR) DVB stick and RTL_433 tool.

byte on1[16] =  {0x8E,0x88,0x8E,0x88,0x8E,0x88,0x8E,0x8E,0x8E,0x8E,0x88,0xEE,0x80,0x00,0x00,0x00};
byte off1[16] = {0x8E,0x88,0x8E,0x88,0x8E,0x88,0x8E,0x8E,0x8E,0x8E,0xEE,0x88,0x80,0x00,0x00,0x00};

byte on4[16] =  {0x8E,0x88,0x8E,0x88,0x8E,0x8E,0x8E,0x8E,0x88,0x8E,0x88,0xEE,0x80,0x00,0x00,0x00};
byte off4[16] = {0x8E,0x88,0x8E,0x88,0x8E,0x8E,0x8E,0x8E,0x88,0x8E,0xEE,0x88,0x80,0x00,0x00,0x00};
 
// This part, take 14 first bytes of 1rst message sent by your Deltia Emitter
byte SunArea1[14] =   {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0xd5,0x15,0x7e,0x1e,0x00,0x3f};
byte MoonArea1[14] =  {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0xd5,0x55,0x7e,0x2b,0xff,0xc0};
byte SunArea2[14] =   {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x2a,0xea,0x81,0xde,0x00,0x3f};
byte MoonArea2[14]=   {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x2a,0xaa,0x81,0xf8,0xff,0xe0};
byte SunArea3[14]=    {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x2a,0xea,0x81,0xf1,0xff,0xc0};
byte MoonArea3[14]=   {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x2a,0xaa,0x81,0xc1,0xff,0xc0};

// 1rst message but only 13 bytes are necessary
byte AssoArea1[13]=   {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0xd5,0x15,0x7e,0x1e,0x00};
byte AssoArea2[13]=   {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x2a,0xea,0x81,0xde,0x00};
byte AssoArea3[13]=   {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x2a,0xea,0x81,0xf1,0xff};


// This part, take 14 first bytes of 2nd message sent by your Deltia Emitter
byte OffArea1[14]=    {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0x95,0x4a,0x81,0xf1,0xff,0xc0};
byte OffArea2[14]=    {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x6a,0xb5,0x7e,0x31,0xff,0xc0};
byte OffArea3[14]=    {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x6a,0xb5,0x7e,0x11,0xff,0xc0};

byte OnArea1[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0x95,0x0a,0x81,0xd1,0xff,0xc0};
byte OnArea2[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x6a,0xf5,0x7e,0x04,0xff,0xe0};
byte OnArea3[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x6a,0xf5,0x7e,0x39,0xff,0xc0};

byte HgArea1[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0xaa,0xca,0xad,0x95,0x35,0x7e,0x31,0xff,0xc0};
byte HgArea2[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0xd5,0x35,0x52,0x6a,0xca,0x81,0xee,0x00,0x3f};
byte HgArea3[14]=     {0x55,0x7f,0x7b,0x0e,0xca,0x95,0x35,0x52,0x6a,0xca,0x81,0xd1,0xff,0xc0};