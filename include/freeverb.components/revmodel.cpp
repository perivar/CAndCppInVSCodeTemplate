// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#include "revmodel.hpp"

revmodel::revmodel()
{
  numcombs = 0;
  numallpasses = 0;
  currentfs = defaultfs;
  setFilter(1);
  setDefault();
  // Buffer will be full of rubbish - so we MUST mute them
  mute();
}

revmodel::~revmodel()
{
  freeFilter();
}

void revmodel::resetfs(int fs)
{
  currentfs = fs;
  float tf = (float)fs/(float)defaultfs;
#ifdef DEBUG
  fprintf(stderr, "freeverb: Fs=%d[Hz] Factor=%f\n", fs, tf);
#endif
  setFilter(tf);
  setDefault();
  mute();
}

int revmodel::f_(int def, float factor)
{
  return (int)((float)(def)*factor);
}

void revmodel::printconfig()
{
  fprintf(stderr, "*** Freeverb config ***\n");
  fprintf(stderr, "Fs=%d[Hz]\n",currentfs);
  fprintf(stderr, "gain %f roomsize %f roomsize1 %f\n",
	  gain, roomsize,roomsize1);
  fprintf(stderr, "damp %f damp1 %f wet %f wet1 %f wet2 %f\n",
	  damp,damp1,wet,wet1,wet2);
  fprintf(stderr, "dry %f width %f mode %f\n", dry,width,mode);
}

void revmodel::mute()
{
  if (getmode() >= freezemode)
    return;
  
  for (int i=0;i<numcombs;i++)
    {
      combL[i].mute();
      combR[i].mute();
    }
  for (int i=0;i<numallpasses;i++)
    {
      allpassL[i].mute();
      allpassR[i].mute();
    }
}

void revmodel::processreplace(float *inputL, float *inputR, float *outputL, float *outputR,
			      long numsamples, int skip)
{
  process(inputL, inputR, outputL, outputR, numsamples, skip, 'R');
}

void revmodel::processmix(float *inputL, float *inputR, float *outputL, float *outputR,
			  long numsamples, int skip)
{
  process(inputL, inputR, outputL, outputR, numsamples, skip, 'M');
}

void revmodel::process(float *inputL, float *inputR, float *outputL, float *outputR,
		     long numsamples, int skip, char pmode)
{
  float outL,outR,input;
  
  while(numsamples-- > 0)
    {
      outL = outR = 0.0;
      input = (*inputL + *inputR) * gain;
      
      // Accumulate comb filters in parallel
      for(int i=0; i<numcombs; i++)
	{
	  outL += combL[i].process(input);
	  outR += combR[i].process(input);
	}
      
      // Feed through allpasses in series
      for(int i=0; i<numallpasses; i++)
	{
	  outL = allpassL[i].process(outL);
	  outR = allpassR[i].process(outR);
	}
      
      switch(pmode)
	{
	case 'M':
	  // mixing
	  *outputL += outL*wet1 + outR*wet2 + *inputL*dry;
	  *outputR += outR*wet1 + outL*wet2 + *inputR*dry;
	  break;
	case 'R':
	  // replacing
	  *outputL = outL*wet1 + outR*wet2 + *inputL*dry;
	  *outputR = outR*wet1 + outL*wet2 + *inputR*dry;
	  break;
	default:
	  break;
	}
      
      // Increment sample pointers, allowing for interleave (if any)
      inputL += skip;
      inputR += skip;
      outputL += skip;
      outputR += skip;
    }
}

void revmodel::update()
{
  // Recalculate internal values after parameter change
  
  int i;
  
  wet1 = wet*(width/2 + 0.5f);
  wet2 = wet*((1-width)/2);
  
  if (mode >= freezemode)
    {
      roomsize1 = 1;
      damp1 = 0;
      gain = muted;
    }
  else
    {
      roomsize1 = roomsize;
      damp1 = damp;
      gain = fixedgain;
    }
  
  for(i=0; i<numcombs; i++)
    {
      combL[i].setfeedback(roomsize1);
      combR[i].setfeedback(roomsize1);
    }
  
  for(i=0; i<numcombs; i++)
    {
      combL[i].setdamp(damp1);
      combR[i].setdamp(damp1);
    }
}

void revmodel::setroomsize(float value)
{
  roomsize = (value*scaleroom) + offsetroom;
  update();
}

float revmodel::getroomsize()
{
  return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
  damp = value*scaledamp;
  update();
}

float revmodel::getdamp()
{
  return damp/scaledamp;
}

void revmodel::setwet(float value)
{
  wet = value*scalewet;
  update();
}

float revmodel::getwet()
{
  return wet/scalewet;
}

void revmodel::setdry(float value)
{
  dry = value*scaledry;
}

float revmodel::getdry()
{
  return dry/scaledry;
}

void revmodel::setwidth(float value)
{
  width = value;
  update();
}

float revmodel::getwidth()
{
  return width;
}

void revmodel::setmode(float value)
{
  mode = value;
  update();
}

float revmodel::getmode()
{
  if (mode >= freezemode)
    return 1;
  else
    return 0;
}

void revmodel::allocFilter(int ncomb, int nallpass)
{
  if(numcombs != 0||numallpasses != 0)
    freeFilter();
  combL = new comb[ncomb];
  combR = new comb[ncomb];
  allpassL = new allpass[nallpass];
  allpassR = new allpass[nallpass];
  numcombs = ncomb;
  numallpasses = nallpass;
}

void revmodel::freeFilter()
{
  if(numcombs != 0&&numallpasses != 0)
    {
#ifdef DEBUG
  fprintf(stderr, "revmodel::freeFilter()+\n");
#endif
      delete[] combL;
      delete[] combR;
      delete[] allpassL;
      delete[] allpassR;
      numcombs = 0;
      numallpasses = 0;
    }
}

void revmodel::setFilter(float factor)
{
#ifdef DEBUG
  fprintf(stderr, "revmodel::setBuffer(%f)\n", factor);
#endif
  const int combT[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
  const int allpT[] = {556, 441, 341, 225};
  allocFilter(sizeof(combT)/sizeof(int), sizeof(allpT)/sizeof(int));
  for(int i=0; i<numcombs; i++)
    {
      combL[i].setsize(f_(combT[i],factor));
      combR[i].setsize(f_(combT[i]+stereospread,factor));
    }
  for(int i=0; i<numallpasses; i++)
    {
      allpassL[i].setsize(f_(allpT[i],factor));
      allpassR[i].setsize(f_(allpT[i]+stereospread,factor));
    }
}

void revmodel::setDefault()
{
  for(int i=0; i<numallpasses; i++)
    {
      allpassL[i].setfeedback(0.5f);
      allpassR[i].setfeedback(0.5f);
    }
  setwet(initialwet);
  setroomsize(initialroom);
  setdry(initialdry);
  setdamp(initialdamp);
  setwidth(initialwidth);
  setmode(initialmode);
}

//ends
