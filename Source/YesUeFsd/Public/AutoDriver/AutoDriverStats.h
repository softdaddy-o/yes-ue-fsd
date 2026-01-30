// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Stats/Stats2.h"

/**
 * Performance statistics for Auto Driver system
 *
 * View in-game with console commands:
 * - stat AutoDriver
 * - stat AutoDriverDetailed
 */

// Stat group declarations
DECLARE_STATS_GROUP(TEXT("AutoDriver"), STATGROUP_AutoDriver, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("AutoDriverDetailed"), STATGROUP_AutoDriverDetailed, STATCAT_Advanced);

// ========================================
// Command Execution Stats
// ========================================

/** Time spent executing commands per frame */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Command Execution"), STAT_AutoDriver_CommandExecution, STATGROUP_AutoDriver, YESUEFSD_API);

/** Time spent ticking commands */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Command Tick"), STAT_AutoDriver_CommandTick, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Number of active commands */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Commands"), STAT_AutoDriver_ActiveCommands, STATGROUP_AutoDriver, YESUEFSD_API);

// ========================================
// Navigation Stats
// ========================================

/** Time spent on navigation queries */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Navigation Queries"), STAT_AutoDriver_NavigationQuery, STATGROUP_AutoDriver, YESUEFSD_API);

/** Time spent on path finding */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Path Finding"), STAT_AutoDriver_PathFinding, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Navigation cache hit rate */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Nav Cache Hits"), STAT_AutoDriver_NavCacheHits, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Navigation cache miss rate */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Nav Cache Misses"), STAT_AutoDriver_NavCacheMisses, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Navigation cache entries */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Nav Cache Entries"), STAT_AutoDriver_NavCacheEntries, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

// ========================================
// AI Controller Stats
// ========================================

/** Number of AI controllers created */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("AI Controllers Created"), STAT_AutoDriver_AIControllersCreated, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Number of AI controllers reused */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("AI Controllers Reused"), STAT_AutoDriver_AIControllersReused, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Number of active AI controllers */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active AI Controllers"), STAT_AutoDriver_ActiveAIControllers, STATGROUP_AutoDriver, YESUEFSD_API);

// ========================================
// HTTP Server Stats
// ========================================

/** Time spent processing HTTP requests */
DECLARE_CYCLE_STAT_EXTERN(TEXT("HTTP Request Processing"), STAT_AutoDriver_HTTPProcessing, STATGROUP_AutoDriver, YESUEFSD_API);

/** HTTP requests per second */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("HTTP Requests"), STAT_AutoDriver_HTTPRequests, STATGROUP_AutoDriver, YESUEFSD_API);

/** HTTP response time (average) */
DECLARE_FLOAT_COUNTER_STAT_EXTERN(TEXT("HTTP Response Time (ms)"), STAT_AutoDriver_HTTPResponseTime, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

// ========================================
// Memory Stats
// ========================================

/** Memory used by command queue */
DECLARE_MEMORY_STAT_EXTERN(TEXT("Command Queue Memory"), STAT_AutoDriver_CommandQueueMemory, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Memory used by navigation cache */
DECLARE_MEMORY_STAT_EXTERN(TEXT("Navigation Cache Memory"), STAT_AutoDriver_NavCacheMemory, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Memory used by recording buffer */
DECLARE_MEMORY_STAT_EXTERN(TEXT("Recording Buffer Memory"), STAT_AutoDriver_RecordingMemory, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

// ========================================
// Recording Stats
// ========================================

/** Time spent recording actions */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Action Recording"), STAT_AutoDriver_Recording, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

/** Number of recorded actions */
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Recorded Actions"), STAT_AutoDriver_RecordedActions, STATGROUP_AutoDriverDetailed, YESUEFSD_API);

// ========================================
// Input Simulation Stats
// ========================================

/** Time spent simulating input */
DECLARE_CYCLE_STAT_EXTERN(TEXT("Input Simulation"), STAT_AutoDriver_InputSimulation, STATGROUP_AutoDriverDetailed, YESUEFSD_API);
