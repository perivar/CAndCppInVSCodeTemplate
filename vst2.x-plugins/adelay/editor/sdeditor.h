//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/11/13 09:08:28 $
//
// Category     : VST 2.x SDK Samples
// Filename     : sdeditor.h
// Created by   : Steinberg Media Technologies
// Description  : Simple Surround Delay plugin with Editor using VSTGUI
//
// � 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __sdeditor__
#define __sdeditor__

#ifndef __aeffguieditor__
#include "../../vstgui4/vstgui/plugin-bindings/aeffguieditor.h"
#endif

//-----------------------------------------------------------------------------
class SDEditor : public VSTGUI::AEffGUIEditor, public VSTGUI::IControlListener
{
public:
	SDEditor (AudioEffect* effect);
	virtual ~SDEditor ();

public:
	virtual bool open (void* ptr);
	virtual void close ();

	virtual void setParameter (VstInt32 index, float value);
	virtual void valueChanged (VSTGUI::CDrawContext* context, VSTGUI::CControl* control);

private:
	// Controls
	VSTGUI::CVerticalSlider* delayFader;
	VSTGUI::CVerticalSlider* feedbackFader;
	VSTGUI::CVerticalSlider* volumeFader;

	VSTGUI::CParamDisplay* delayDisplay;
	VSTGUI::CParamDisplay* feedbackDisplay;
	VSTGUI::CParamDisplay* volumeDisplay;

	// Bitmap
	VSTGUI::CBitmap* hBackground;
};

#endif