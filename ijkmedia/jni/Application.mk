APP_OPTIM := release
APP_PLATFORM := android-9
APP_ABI := armeabi-v7a
NDK_TOOLCHAIN_VERSION=4.7
APP_PIE := false

APP_CFLAGS := -std=c99
APP_CFLAGS += -O3 -Wall -pipe \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -fmodulo-sched -fmodulo-sched-allow-regmoves \
    -Wno-psabi -Wa,--noexecstack \
    -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ \
    -DANDROID -DNDEBUG

APP_CFLAGS += -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp
APP_LDFLAGS += -Wl,--fix-cortex-a8