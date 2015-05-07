#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <sys/param.h>
#import <mach/mach_host.h>
#import <sys/sysctl.h>
#include <mach/mach_time.h>
#include <CoreVideo/CVHostTime.h>

#import "iOSUtils.h"

float GetIOSVersion(void)
{
    @autoreleasepool
    {
        return [[[UIDevice currentDevice] systemVersion] floatValue];
    }
}

bool HasVideoToolboxDecoder(void)
{
    static int DecoderAvailable = -1;
    
    if (DecoderAvailable == -1)
    {
        if(GetIOSVersion()>=8.0)
        {
            DecoderAvailable = 1;
        }else{
            if (IsIosSandboxed())
            {
                uint64_t proc_enforce = 0;
                uint64_t vnode_enforce = 0;
                size_t size = sizeof(vnode_enforce);
                
                sysctlbyname("security.mac.proc_enforce",  &proc_enforce,  &size, NULL, 0);
                sysctlbyname("security.mac.vnode_enforce", &vnode_enforce, &size, NULL, 0);
                
                if (vnode_enforce && proc_enforce)
                {
                    DecoderAvailable = 0;
                    printf("VideoToolBox decoder not available. Use : sysctl -w security.mac.proc_enforce=0; sysctl -w security.mac.vnode_enforce=0\n");
                }
                else
                {
                    DecoderAvailable = 1;
                    printf("VideoToolBox decoder available\n");
                }
            }
            else
            {
                DecoderAvailable = 1;
            }
        }
    }
    
    return (DecoderAvailable == 1);
}

int GetExecutablePath(char* path, uint32_t *pathsize)
{
    @autoreleasepool
    {
        NSString *pathname;
        
        pathname = [[NSBundle mainBundle] executablePath];
        strcpy(path, [pathname UTF8String]);
        *pathsize = strlen(path);
        
        return 0;
    }
}

bool IsIosSandboxed(void)
{
    static int ret = -1;
    if (ret == -1)
    {
        uint32_t path_size = 2*MAXPATHLEN;
        char     given_path[2*MAXPATHLEN];
        int      result = -1;
        ret = 0;
        memset(given_path, 0x0, path_size);
        /* Get Application directory */
        result = GetExecutablePath(given_path, &path_size);
        if (result == 0)
        {
            // we re sandboxed if we are installed in /var/mobile/Applications
            if (strlen("/var/mobile/Applications/") < path_size &&
                strncmp(given_path, "/var/mobile/Applications/", strlen("/var/mobile/Applications/")) == 0)
            {
                ret = 1;
            }
        }
    }
    return ret == 1;
}

// platform strings are based on http://theiphonewiki.com/wiki/Models
const char *getIosPlatformString(void)
{
    static char iOSPlatformString[128]={'\0'};
    if (iOSPlatformString[0]=='\0')
    {
        // Gets a string with the device model
        size_t size;
        sysctlbyname("hw.machine", NULL, &size, NULL, 0);
        char *machine = (char*)malloc(size);
        if (sysctlbyname("hw.machine", machine, &size, NULL, 0) == 0 && machine[0])
            strlcpy(iOSPlatformString, machine, sizeof(iOSPlatformString));
        else
            strlcpy(iOSPlatformString, "unknown0,0", sizeof(iOSPlatformString));
        
        free(machine);
    }
    
    return iOSPlatformString;
}

enum iosPlatform getIosPlatform()
{
    static enum iosPlatform eDev = iDeviceUnknown;
    
    if (eDev == iDeviceUnknown)
    {
        const char* devStr = getIosPlatformString();
        
        if (!strcmp(devStr, "iPhone1,1")) eDev = iPhone2G;
        else if (!strcmp(devStr, "iPhone1,2")) eDev = iPhone3G;
        else if (!strcmp(devStr, "iPhone2,1")) eDev = iPhone3GS;
        else if (!strcmp(devStr, "iPhone3,1")) eDev = iPhone4;
        else if (!strcmp(devStr, "iPhone3,2")) eDev = iPhone4;
        else if (!strcmp(devStr, "iPhone3,3")) eDev = iPhone4CDMA;
        else if (!strcmp(devStr, "iPhone4,1")) eDev = iPhone4S;
        else if (!strcmp(devStr, "iPhone5,1")) eDev = iPhone5;
        else if (!strcmp(devStr, "iPhone5,2")) eDev = iPhone5GSMCDMA;
        else if (!strcmp(devStr, "iPhone5,3")) eDev = iPhone5CGSM;
        else if (!strcmp(devStr, "iPhone5,4")) eDev = iPhone5CGlobal;
        else if (!strcmp(devStr, "iPhone6,1")) eDev = iPhone5SGSM;
        else if (!strcmp(devStr, "iPhone6,2")) eDev = iPhone5SGlobal;
        else if (!strcmp(devStr, "iPhone7,1")) eDev = iPhone6Plus;
        else if (!strcmp(devStr, "iPhone7,2")) eDev = iPhone6;
        else if (!strcmp(devStr, "iPod1,1")) eDev = iPodTouch1G;
        else if (!strcmp(devStr, "iPod2,1")) eDev = iPodTouch2G;
        else if (!strcmp(devStr, "iPod3,1")) eDev = iPodTouch3G;
        else if (!strcmp(devStr, "iPod4,1")) eDev = iPodTouch4G;
        else if (!strcmp(devStr, "iPod5,1")) eDev = iPodTouch5G;
        else if (!strcmp(devStr, "iPad1,1")) eDev = iPad;
        else if (!strcmp(devStr, "iPad1,2")) eDev = iPad;
        else if (!strcmp(devStr, "iPad2,1")) eDev = iPad2WIFI;
        else if (!strcmp(devStr, "iPad2,2")) eDev = iPad2;
        else if (!strcmp(devStr, "iPad2,3")) eDev = iPad2CDMA;
        else if (!strcmp(devStr, "iPad2,4")) eDev = iPad2;
        else if (!strcmp(devStr, "iPad2,5")) eDev = iPadMiniWIFI;
        else if (!strcmp(devStr, "iPad2,6")) eDev = iPadMini;
        else if (!strcmp(devStr, "iPad2,7")) eDev = iPadMiniGSMCDMA;
        else if (!strcmp(devStr, "iPad3,1")) eDev = iPad3WIFI;
        else if (!strcmp(devStr, "iPad3,2")) eDev = iPad3GSMCDMA;
        else if (!strcmp(devStr, "iPad3,3")) eDev = iPad3;
        else if (!strcmp(devStr, "iPad3,4")) eDev = iPad4WIFI;
        else if (!strcmp(devStr, "iPad3,5")) eDev = iPad4;
        else if (!strcmp(devStr, "iPad3,6")) eDev = iPad4GSMCDMA;
        else if (!strcmp(devStr, "iPad4,1")) eDev = iPadAirWifi;
        else if (!strcmp(devStr, "iPad4,2")) eDev = iPadAirCellular;
        else if (!strcmp(devStr, "iPad4,4")) eDev = iPadMini2Wifi;
        else if (!strcmp(devStr, "iPad4,5")) eDev = iPadMini2Cellular;
        else if (!strcmp(devStr, "iPad4,7")) eDev = iPadMini3Wifi;
        else if (!strcmp(devStr, "iPad4,8")) eDev = iPadMini3Cellular;
        else if (!strcmp(devStr, "iPad4,9")) eDev = iPadMini3Cellular;
        else if (!strcmp(devStr, "iPad5,3")) eDev = iPadAir2Wifi;
        else if (!strcmp(devStr, "iPad5,4")) eDev = iPadAir2Cellular;
        else if (!strcmp(devStr, "AppleTV2,1")) eDev = AppleTV2;
    }

    return eDev;
}