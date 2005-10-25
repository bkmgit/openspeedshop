/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/

#ifndef SS_INPUT_MANAGER_H
#define SS_INPUT_MANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>               /* for fstat() */
#include <fcntl.h>
#include <sys/mman.h>               /* for mmap() */
#include <time.h>
#include <stdio.h>
#include <list>
#include <inttypes.h>
#include <stdexcept>
#include <string>

#include <vector>
#include <iostream>

#ifndef PTHREAD_MUTEX_RECURSIVE_NP
#define PTHREAD_MUTEX_RECURSIVE_NP 0
#endif

// for host name description
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <netdb.h>

// for catching hard interrupts and interprocess signals
#include <sys/wait.h>
#define SET_SIGNAL(s, f)                              \
  if (signal (s, SIG_IGN) != SIG_IGN)                 \
    signal (s, reinterpret_cast <void (*)(int)> (f));

using namespace std;

#include "ToolAPI.hxx"

using namespace OpenSpeedShop;
using namespace OpenSpeedShop::Framework;

#include "SS_Configure.hxx"
#include "Commander.hxx"

#include "SS_Message_Element.hxx"
#include "SS_Message_Czar.hxx"

extern SS_Message_Czar& theMessageCzar();

#include "SS_Output.hxx"

#include "SS_Parse_Range.hxx"
#include "SS_Parse_Target.hxx"
#include "SS_Parse_Result.hxx"

#include "SS_Cmd_Control.hxx"
#include "SS_Cmd_Execution.hxx"
#include "CommandObject.hxx"
#include "Clip.hxx"
#include "SS_Exp.hxx"
#include "SS_View.hxx"

#include "SS_Watcher.hxx"

#include "ArgClass.hxx"

using namespace OpenSpeedShop::cli;

#endif // SS_INPUT_MANAGER_H
