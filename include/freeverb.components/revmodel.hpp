// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#ifndef _revmodel_
#define _revmodel_

#include <stdio.h>
#include "comb.hpp"
#include "allpass.hpp"
#include "tuning.h"

class revmodel
{
public:
  revmodel();
  ~revmodel();
  void  resetfs(int fs);
  void  setDefault();
  void	mute();
  void	processmix(float *inputL, float *inputR,
		   float *outputL, float *outputR, long numsamples, int skip);
  void	processreplace(float *inputL, float *inputR,
		       float *outputL, float *outputR, long numsamples, int skip);
  void	setroomsize(float value);
  float	getroomsize();
  void	setdamp(float value);
  float	getdamp();
  void	setwet(float value);
  float	getwet();
  void	setdry(float value);
  float	getdry();
  void	setwidth(float value);
  float	getwidth();
  void	setmode(float value);
  float	getmode();
  void  printconfig();
private:
  void process(float *inputL, float *inputR, float *outputL, float *outputR,
	       long numsamples, int skip, char pmode);
  int   currentfs;
  int   f_(int def, float factor);
  void	update();
  void  allocFilter(int ncomb, int nallpass);
  void  freeFilter();
  void  setFilter(float factor);
  float	gain;
  float	roomsize,roomsize1;
  float	damp,damp1;
  float	wet,wet1,wet2;
  float	dry;
  float	width;
  float	mode;  
  int   numcombs;
  int   numallpasses;
  comb *combL;
  comb *combR;
  allpass *allpassL;
  allpass *allpassR;
};

#endif//_revmodel_

//ends
