// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Version string for the YesUeFsd plugin
 * Format: MAJOR.MINOR.PATCH
 * - MAJOR: Breaking changes
 * - MINOR: New features
 * - PATCH: Bug fixes
 */
#define YESUEFSD_VERSION TEXT("0.1.0")

/**
 * YesUeFsd Runtime Module
 *
 * Provides core functionality for automatic player driving and gameplay automation.
 * This module contains runtime-accessible classes and utilities that can be used
 * in both editor and packaged game builds.
 */
class FYesUeFsdModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Get the version string for this plugin
	 * @return Version string in format "MAJOR.MINOR.PATCH"
	 */
	static FString GetVersion() { return YESUEFSD_VERSION; }
};
