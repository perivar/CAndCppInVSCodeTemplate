// Reverb model tuning values
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _tuning_
#define _tuning_

const float muted		= 0.0f;
const float fixedgain		= 0.015f;
const float scalewet		= 3.0f;
const float scaledry		= 2.0f;
const float scaledamp		= 0.4f;
const float scaleroom		= 0.28f;
const float offsetroom		= 0.7f;
const float initialroom		= 0.5f;
const float initialdamp		= 0.5f;
const float initialwet	        = 1/scalewet;
const float initialdry		= 0.0f;
const float initialwidth	= 1.0f;
const float initialmode		= 0.0f;
const float freezemode		= 0.5f;
const int stereospread	= 23;
const int defaultfs             = 44100;

#endif
