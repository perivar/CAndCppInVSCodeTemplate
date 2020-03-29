// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain
// Fixed by Teru KAMOGASHIRA

#include "comb.hpp"

comb::comb()
{
  bufsize = 0;
  filterstore = 0;
  bufidx = 0;
}

comb::~comb()
{
  if(bufsize != 0)
    delete[] buffer;
}

int comb::getsize()
{
  return bufsize;
}

void comb::setsize(int size) 
{
#ifdef DEBUG
  fprintf(stderr, "comb::setsize(%d)\n", size);
#endif
  if(bufsize != 0)
    delete[] buffer;
  buffer = new float[size];
  bufsize = size;
  bufidx = 0;
}

void comb::mute()
{
  for (int i=0; i<bufsize; i++)
    buffer[i]=0;
}

void comb::setdamp(float val) 
{
  damp1 = val; 
  damp2 = 1-val;
}

float comb::getdamp() 
{
  return damp1;
}

void comb::setfeedback(float val) 
{
  feedback = val;
}

float comb::getfeedback() 
{
  return feedback;
}

float comb::process(float input)
{
  float output;
  
  output = buffer[bufidx];
  filterstore = (output*damp2) + (filterstore*damp1);
  buffer[bufidx] = input + (filterstore*feedback);
  
  bufidx ++;
  if(bufidx >= bufsize)
    bufidx = 0;
  
  return output;
}

// ends
