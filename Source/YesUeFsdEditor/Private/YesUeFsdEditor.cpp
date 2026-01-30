// Copyright Epic Games, Inc. All Rights Reserved.

#include "YesUeFsdEditor.h"
#include "YesUeFsd.h"

#define LOCTEXT_NAMESPACE "FYesUeFsdEditorModule"

void FYesUeFsdEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogTemp, Log, TEXT("YesUeFsd Editor Module Started - Version %s"), *FYesUeFsdModule::GetVersion());

	// Register menu extensions
	RegisterMenuExtensions();
}

void FYesUeFsdEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unregister menu extensions
	UnregisterMenuExtensions();

	UE_LOG(LogTemp, Log, TEXT("YesUeFsd Editor Module Shutdown"));
}

void FYesUeFsdEditorModule::RegisterMenuExtensions()
{
	// TODO: Register editor menu extensions for manual control UI
}

void FYesUeFsdEditorModule::UnregisterMenuExtensions()
{
	// TODO: Unregister editor menu extensions
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FYesUeFsdEditorModule, YesUeFsdEditor)
