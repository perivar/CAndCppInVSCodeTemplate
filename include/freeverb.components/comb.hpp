// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#ifndef _comb_
#define _comb_

#include <stdio.h>

class comb
{
public:
  comb();
  ~comb();
  void	setsize(int size);
  int   getsize();
  float	process(float inp);
  void	mute();
  void	setdamp(float val);
  float	getdamp();
  void	setfeedback(float val);
  float	getfeedback();
private:
  float	*buffer;
  float	feedback;
  float	filterstore;
  float	damp1;
  float	damp2;
  int bufsize;
  int bufidx;
};

#endif

