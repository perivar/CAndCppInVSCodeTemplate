// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_ABB2F24A929B3627B860CAAB4ABDE216
#define AU_COCOA_VIEW_NAME AUCocoaView_ABB2F24A929B3627B860CAAB4ABDE216

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.demosynthplugin.bundleID";
const char* kAUBundleID = "developer.au.demosynthplugin.bundleID";
const char* kVST3BundleID = "developer.vst3.demosynthplugin.bundleID";

// --- Plugin Names 
const char* kPluginName = "DemoSynthPlugin";
const char* kShortPluginName = "DemoSynthPlugin";
const char* kAUBundleName = "DemoSynthPlugin";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kSynthPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{abb2f24a-929b-3627-b860-caab4abde216}";

// --- 4-char codes 
const int32_t kFourCharCode = 'dsp1';
const int32_t kAAXProductID = 'dsp1';
const int32_t kManufacturerID = 'ASPK';

// --- Vendor information 
const char* kVendorName = "ASPiK'";
const char* kVendorURL = "www.myplugins.com";
const char* kVendorEmail = "support@myplugins.com";

// --- Plugin Options 
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 1;
const uint32_t kAAXCategory = 2048;

#endif
