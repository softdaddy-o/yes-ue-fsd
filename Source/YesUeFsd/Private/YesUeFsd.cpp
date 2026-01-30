// Copyright Epic Games, Inc. All Rights Reserved.

#include "YesUeFsd.h"

#define LOCTEXT_NAMESPACE "FYesUeFsdModule"

void FYesUeFsdModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogTemp, Log, TEXT("YesUeFsd Runtime Module Started - Version %s"), *GetVersion());
}

void FYesUeFsdModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Log, TEXT("YesUeFsd Runtime Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FYesUeFsdModule, YesUeFsd)
