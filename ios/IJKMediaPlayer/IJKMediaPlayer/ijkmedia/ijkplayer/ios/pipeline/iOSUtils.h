#ifndef MediaPlayer_iOSUtils_h
#define MediaPlayer_iOSUtils_h

// We forward declare CFStringRef in order to avoid
// pulling in tons of Objective-C headers.
struct __CFString;
typedef const struct __CFString * CFStringRef;

enum iosPlatform
{
    iDeviceUnknown = -1,
    iPhone2G,
    iPhone3G,
    iPhone3GS,
    iPodTouch1G,
    iPodTouch2G,
    iPodTouch3G,
    iPad,
    iPad3G,
    iPad2WIFI,
    iPad2CDMA,
    iPad2,
    iPadMini,
    iPadMiniGSMCDMA,
    iPadMiniWIFI,
    AppleTV2,
    iPhone4,            //from here on list devices with retina support (e.x. mainscreen scale == 2.0)
    iPhone4CDMA,
    iPhone4S,
    iPhone5,
    iPhone5GSMCDMA,
    iPhone5CGSM,
    iPhone5CGlobal,
    iPhone5SGSM,
    iPhone5SGlobal,
    iPodTouch4G,
    iPodTouch5G,
    iPad3WIFI,
    iPad3GSMCDMA,
    iPad3,
    iPad4WIFI,
    iPad4,
    iPad4GSMCDMA,
    iPadAirWifi,
    iPadAirCellular,
    iPadMini2Wifi,
    iPadMini2Cellular,
    iPhone6,
    iPadAir2Wifi,
    iPadAir2Cellular,
    iPadMini3Wifi,
    iPadMini3Cellular,
    iPhone6Plus,        //from here on list devices with retina support which have scale == 3.0
};


const char *getIosPlatformString(void);
enum iosPlatform getIosPlatform();

int GetExecutablePath(char* path, uint32_t *pathsize);
bool IsIosSandboxed(void);

float GetIOSVersion(void);
bool HasVideoToolboxDecoder(void);

#endif
