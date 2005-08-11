#ifndef __ARSLEXIS_PALM_SYSUTILS_HPP__
#define __ARSLEXIS_PALM_SYSUTILS_HPP__

#include <BaseTypes.hpp>

struct Point;

void sendEvent(uint_t event, const void* data = NULL, uint_t dataSize = 0, bool unique = false, const Point* point = NULL);

template<class EventData>
void sendEvent(uint_t event, const EventData& data, bool unique = false, const Point* point = NULL)
{sendEvent(event, &data, sizeof(data), unique, point);}

void processReadyUiEvents();

UInt32 romVersion();
UInt16 romVersionMajor();
UInt16 romVersionMinor();

char_t* getResource(UInt16 stringId);
char_t* getDataResource(UInt16 dataId, UInt32 *resSizeOut);

void getScreenBounds(RectangleType& bounds);

// detect a web browser app and return cardNo and dbID of its *.prc.
// returns true if detected some viewer, false if none was found
bool fDetectViewer(UInt16 *cardNoOut, LocalID *dbIDOut);

Err getResource(UInt16 tableId, UInt16 index, String& out);

Err WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, const char *parameterP, UInt32 *resultP);

bool highDensityFeaturesPresent();

bool notifyManagerPresent();

bool selectDate(const char* title, Int16* month, Int16* day, Int16* year);

void getDate(Int16* month, Int16* day, Int16* year);

// assert that outBuffer length is more than needed
void localizeDate(char* outBuffer, Int16 month, Int16 day, Int16 year);

bool selectTime(const char* title, Int16* hour, Int16* minutes);

void getTime(Int16* hour, Int16* minutes);

// assert that outBuffer length is more than needed
void localizeTime(char* outBuffer, Int16 hour, Int16 minutes);

bool VFS_FeaturesPresent();

#endif
