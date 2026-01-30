// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/AutoDriverStats.h"

// Define all the stats declared in the header

// Command Execution
DEFINE_STAT(STAT_AutoDriver_CommandExecution);
DEFINE_STAT(STAT_AutoDriver_CommandTick);
DEFINE_STAT(STAT_AutoDriver_ActiveCommands);

// Navigation
DEFINE_STAT(STAT_AutoDriver_NavigationQuery);
DEFINE_STAT(STAT_AutoDriver_PathFinding);
DEFINE_STAT(STAT_AutoDriver_NavCacheHits);
DEFINE_STAT(STAT_AutoDriver_NavCacheMisses);
DEFINE_STAT(STAT_AutoDriver_NavCacheEntries);

// AI Controllers
DEFINE_STAT(STAT_AutoDriver_AIControllersCreated);
DEFINE_STAT(STAT_AutoDriver_AIControllersReused);
DEFINE_STAT(STAT_AutoDriver_ActiveAIControllers);

// HTTP Server
DEFINE_STAT(STAT_AutoDriver_HTTPProcessing);
DEFINE_STAT(STAT_AutoDriver_HTTPRequests);
DEFINE_STAT(STAT_AutoDriver_HTTPResponseTime);

// Memory
DEFINE_STAT(STAT_AutoDriver_CommandQueueMemory);
DEFINE_STAT(STAT_AutoDriver_NavCacheMemory);
DEFINE_STAT(STAT_AutoDriver_RecordingMemory);

// Recording
DEFINE_STAT(STAT_AutoDriver_Recording);
DEFINE_STAT(STAT_AutoDriver_RecordedActions);

// Input Simulation
DEFINE_STAT(STAT_AutoDriver_InputSimulation);
