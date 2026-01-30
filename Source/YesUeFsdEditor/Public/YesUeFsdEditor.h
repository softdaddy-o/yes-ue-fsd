// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * YesUeFsd Editor Module
 *
 * Provides editor-time functionality for automatic player driving:
 * - HTTP server for remote control
 * - Editor UI for manual control and testing
 * - PIE (Play-In-Editor) integration for automated testing
 * - Recording and playback of player actions
 */
class FYesUeFsdEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register editor UI extensions */
	void RegisterMenuExtensions();

	/** Unregister editor UI extensions */
	void UnregisterMenuExtensions();
};
