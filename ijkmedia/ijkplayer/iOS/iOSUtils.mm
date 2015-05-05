//
//  iOSUtils.m
//  MediaPlayer
//
//  Created by 施灵凯 on 15/4/24.
//  Copyright (c) 2015年 bolome. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <sys/param.h>
#import <mach/mach_host.h>
#import <sys/sysctl.h>
#include <mach/mach_time.h>
#include <CoreVideo/CVHostTime.h>

#import "iOSUtils.h"
#include "Media_Log.h"

float iOSUtils::GetIOSVersion(void)
{
    @autoreleasepool
    {
        return [[[UIDevice currentDevice] systemVersion] floatValue];
    }
}

bool iOSUtils::HasVideoToolboxDecoder(void)
{
    static int DecoderAvailable = -1;
    
    if (DecoderAvailable == -1)
    {
        if(GetIOSVersion()>=8.0)
        {
            DecoderAvailable = 1;
        }else{
            /* When XBMC is started from a sandbox directory we have to check the sysctl values */
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
                    LOGI("VideoToolBox decoder not available. Use : sysctl -w security.mac.proc_enforce=0; sysctl -w security.mac.vnode_enforce=0\n");
                }
                else
                {
                    DecoderAvailable = 1;
                    LOGI("VideoToolBox decoder available\n");
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

int iOSUtils::GetExecutablePath(char* path, uint32_t *pathsize)
{
    @autoreleasepool
    {
        // see if we can figure out who we are
        NSString *pathname;
        
        // a) Kodi frappliance running under ATV2
        // todo
        
        // b) Kodi application running under IOS
        // c) Kodi application running under OSX
        pathname = [[NSBundle mainBundle] executablePath];
        strcpy(path, [pathname UTF8String]);
        *pathsize = strlen(path);
        //CLog::Log(LOGDEBUG, "DarwinExecutablePath(b/c) -> %s", path);
        
        return 0;
    }
}

bool iOSUtils::IsIosSandboxed(void)
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

int64_t iOSUtils::CurrentHostCounter(void)
{
    return( (int64_t)CVGetCurrentHostTime() );
}

int64_t iOSUtils::CurrentHostFrequency(void)
{
    return( (int64_t)CVGetHostClockFrequency() );
}

// platform strings are based on http://theiphonewiki.com/wiki/Models
const char *iOSUtils::getIosPlatformString(void)
{
    static std::string iOSPlatformString;
    if (iOSPlatformString.empty())
    {
        // Gets a string with the device model
        size_t size;
        sysctlbyname("hw.machine", NULL, &size, NULL, 0);
        char *machine = new char[size];
        if (sysctlbyname("hw.machine", machine, &size, NULL, 0) == 0 && machine[0])
            iOSPlatformString.assign(machine, size -1);
        else
            iOSPlatformString = "unknown0,0";
        
        delete [] machine;
    }
    
    return iOSPlatformString.c_str();
}

enum iosPlatform iOSUtils::getIosPlatform()
{
    static enum iosPlatform eDev = iDeviceUnknown;
    
    if (eDev == iDeviceUnknown)
    {
        std::string devStr(iOSUtils::getIosPlatformString());
        
        if (devStr == "iPhone1,1") eDev = iPhone2G;
        else if (devStr == "iPhone1,2") eDev = iPhone3G;
        else if (devStr == "iPhone2,1") eDev = iPhone3GS;
        else if (devStr == "iPhone3,1") eDev = iPhone4;
        else if (devStr == "iPhone3,2") eDev = iPhone4;
        else if (devStr == "iPhone3,3") eDev = iPhone4CDMA;
        else if (devStr == "iPhone4,1") eDev = iPhone4S;
        else if (devStr == "iPhone5,1") eDev = iPhone5;
        else if (devStr == "iPhone5,2") eDev = iPhone5GSMCDMA;
        else if (devStr == "iPhone5,3") eDev = iPhone5CGSM;
        else if (devStr == "iPhone5,4") eDev = iPhone5CGlobal;
        else if (devStr == "iPhone6,1") eDev = iPhone5SGSM;
        else if (devStr == "iPhone6,2") eDev = iPhone5SGlobal;
        else if (devStr == "iPhone7,1") eDev = iPhone6Plus;
        else if (devStr == "iPhone7,2") eDev = iPhone6;
        else if (devStr == "iPod1,1") eDev = iPodTouch1G;
        else if (devStr == "iPod2,1") eDev = iPodTouch2G;
        else if (devStr == "iPod3,1") eDev = iPodTouch3G;
        else if (devStr == "iPod4,1") eDev = iPodTouch4G;
        else if (devStr == "iPod5,1") eDev = iPodTouch5G;
        else if (devStr == "iPad1,1") eDev = iPad;
        else if (devStr == "iPad1,2") eDev = iPad;
        else if (devStr == "iPad2,1") eDev = iPad2WIFI;
        else if (devStr == "iPad2,2") eDev = iPad2;
        else if (devStr == "iPad2,3") eDev = iPad2CDMA;
        else if (devStr == "iPad2,4") eDev = iPad2;
        else if (devStr == "iPad2,5") eDev = iPadMiniWIFI;
        else if (devStr == "iPad2,6") eDev = iPadMini;
        else if (devStr == "iPad2,7") eDev = iPadMiniGSMCDMA;
        else if (devStr == "iPad3,1") eDev = iPad3WIFI;
        else if (devStr == "iPad3,2") eDev = iPad3GSMCDMA;
        else if (devStr == "iPad3,3") eDev = iPad3;
        else if (devStr == "iPad3,4") eDev = iPad4WIFI;
        else if (devStr == "iPad3,5") eDev = iPad4;
        else if (devStr == "iPad3,6") eDev = iPad4GSMCDMA;
        else if (devStr == "iPad4,1") eDev = iPadAirWifi;
        else if (devStr == "iPad4,2") eDev = iPadAirCellular;
        else if (devStr == "iPad4,4") eDev = iPadMini2Wifi;
        else if (devStr == "iPad4,5") eDev = iPadMini2Cellular;
        else if (devStr == "iPad4,7") eDev = iPadMini3Wifi;
        else if (devStr == "iPad4,8") eDev = iPadMini3Cellular;
        else if (devStr == "iPad4,9") eDev = iPadMini3Cellular;
        else if (devStr == "iPad5,3") eDev = iPadAir2Wifi;
        else if (devStr == "iPad5,4") eDev = iPadAir2Cellular;
        else if (devStr == "AppleTV2,1") eDev = AppleTV2;
    }

    return eDev;
}

bool iOSUtils::DeviceHasRetina(double &scale)
{
    static enum iosPlatform platform = iDeviceUnknown;
    
    if( platform == iDeviceUnknown )
    {
        platform = getIosPlatform();
    }

    scale = 1.0; //no retina
    
    // see http://www.paintcodeapp.com/news/iphone-6-screens-demystified
    if (platform >= iPhone4 && platform < iPhone6Plus)
    {
        scale = 2.0; // 2x render retina
    }
    
    if (platform >= iPhone6Plus)
    {
        scale = 3.0; //3x render retina + downscale
    }
    
    return (platform >= iPhone4);
}