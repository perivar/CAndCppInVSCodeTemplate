// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#include "allpass.hpp"

allpass::allpass()
{
  bufsize = 0;
  bufidx = 0;
}

allpass::~allpass()
{
  if(bufsize != 0)
    delete[] buffer;
}

int allpass::getsize()
{
  return bufsize;
}

void allpass::setsize(int size) 
{
#ifdef DEBUG
  fprintf(stderr, "allpass::setsize(%d)\n", size);
#endif
  if(bufsize != 0)
    delete[] buffer;
  buffer = new float[size];
  bufsize = size;
  bufidx = 0;
}

void allpass::mute()
{
  for (int i=0; i<bufsize; i++)
    buffer[i]=0;
}

void allpass::setfeedback(float val) 
{
  feedback = val;
}

float allpass::getfeedback() 
{
  return feedback;
}

float allpass::process(float input)
{
  float bufout;
  
  bufout = buffer[bufidx];
  buffer[bufidx] = input + (bufout*feedback);
  
  bufidx ++;
  if(bufidx >= bufsize)
    bufidx = 0;

  return (bufout - input);
}

//ends
