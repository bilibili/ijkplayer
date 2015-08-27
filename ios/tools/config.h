/*
 * config.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if   defined(__aarch64__)
#   include "arm64/config.h"
#elif defined(__x86_64__)
#   include "x86_64/config.h"
#elif defined(__arm__)

#   if   defined(__ARM_ARCH_7S__)
#       include "armv7s/config.h"
#   elif defined(__ARM_ARCH)
#       if __ARM_ARCH == 7
#           include "armv7/config.h"
#       else
#           error Unsupport ARM architecture
#       endif
#   else
#       error Unsupport ARM architecture
#   endif

#elif defined(__i386__)
#   include "i386/config.h"
#else
#   error Unsupport architecture
#endif
