// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#ifndef _allpass_
#define _allpass_
#include <stdio.h>

class allpass
{
public:
  allpass();
  ~allpass();
  void	setsize(int size);
  int   getsize();
  float	process(float inp);
  void	mute();
  void	setfeedback(float val);
  float	getfeedback();
private:
  float	feedback;
  float	*buffer;
  int bufsize;
  int bufidx;
};

#endif

