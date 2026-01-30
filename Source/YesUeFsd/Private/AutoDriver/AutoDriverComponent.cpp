// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "AutoDriver/UIInteractionHelper.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UAutoDriverComponent::UAutoDriverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAutoDriverComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the player controller
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedPlayerController = Cast<APlayerController>(Owner);
		if (!CachedPlayerController)
		{
			// If owner is not a player controller, try to get it from pawn
			if (APawn* OwnerPawn = Cast<APawn>(Owner))
			{
				CachedPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
			}
		}
	}

	if (!CachedPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Could not find PlayerController. Component may not function correctly."));
	}
}

void UAutoDriverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCurrentCommand();
	ReleaseAIController();
	Super::EndPlay(EndPlayReason);
}

void UAutoDriverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnabled)
	{
		return;
	}

	// Update current command
	if (CurrentCommand.IsValid())
	{
		CurrentCommand->Tick(DeltaTime);

		// Check if command is complete
		if (!CurrentCommand->IsRunning())
		{
			FAutoDriverCommandResult Result = CurrentCommand->GetResult();
			OnCommandCompleted(Result);
		}
	}
}

bool UAutoDriverComponent::ExecuteCommand(TScriptInterface<IAutoDriverCommand> Command)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot execute command - component is disabled"));
		return false;
	}

	if (!Command.GetObject())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Invalid command"));
		return false;
	}

	// Stop current command if any
	StopCurrentCommand();

	// Store new command
	IAutoDriverCommand* CommandInterface = Cast<IAutoDriverCommand>(Command.GetObject());
	if (!CommandInterface)
	{
		UE_LOG(LogTemp, Error, TEXT("AutoDriverComponent: Command does not implement IAutoDriverCommand interface"));
		return false;
	}

	// TODO: Convert UObject-based command to TSharedPtr
	// For now, just log a warning
	UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Command system needs implementation"));

	return false;
}

void UAutoDriverComponent::StopCurrentCommand()
{
	if (CurrentCommand.IsValid())
	{
		CurrentCommand->Cancel();
		CurrentCommand.Reset();
	}
}

bool UAutoDriverComponent::MoveToLocation(const FAutoDriverMoveParams& Params)
{
	if (!bEnabled || !CachedPlayerController)
	{
		return false;
	}

	// TODO: Create and execute move command
	UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: MoveToLocation - Target: %s"), *Params.TargetLocation.ToString());

	return false;
}

bool UAutoDriverComponent::MoveToActor(AActor* TargetActor, float AcceptanceRadius)
{
	if (!TargetActor)
	{
		return false;
	}

	FAutoDriverMoveParams Params;
	Params.TargetLocation = TargetActor->GetActorLocation();
	Params.AcceptanceRadius = AcceptanceRadius;

	return MoveToLocation(Params);
}

void UAutoDriverComponent::StopMovement()
{
	StopCurrentCommand();

	// Also stop any ongoing movement on the character
	if (ACharacter* Character = GetControlledCharacter())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}
}

bool UAutoDriverComponent::RotateToRotation(const FAutoDriverRotateParams& Params)
{
	if (!bEnabled || !CachedPlayerController)
	{
		return false;
	}

	// TODO: Create and execute rotation command
	UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: RotateToRotation - Target: %s"), *Params.TargetRotation.ToString());

	return false;
}

bool UAutoDriverComponent::LookAtLocation(FVector TargetLocation, float RotationSpeed)
{
	APawn* ControlledPawn = GetControlledPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	FVector Direction = TargetLocation - ControlledPawn->GetActorLocation();
	FRotator TargetRotation = Direction.Rotation();

	FAutoDriverRotateParams Params;
	Params.TargetRotation = TargetRotation;
	Params.RotationSpeed = RotationSpeed;

	return RotateToRotation(Params);
}

bool UAutoDriverComponent::LookAtActor(AActor* TargetActor, float RotationSpeed)
{
	if (!TargetActor)
	{
		return false;
	}

	return LookAtLocation(TargetActor->GetActorLocation(), RotationSpeed);
}

bool UAutoDriverComponent::PressButton(FName ActionName, float Duration)
{
	if (!bEnabled || !CachedPlayerController)
	{
		return false;
	}

	// TODO: Create and execute input command
	UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: PressButton - Action: %s, Duration: %.2f"), *ActionName.ToString(), Duration);

	return false;
}

bool UAutoDriverComponent::SetAxisValue(FName ActionName, float Value, float Duration)
{
	if (!bEnabled || !CachedPlayerController)
	{
		return false;
	}

	// TODO: Create and execute axis input command
	UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: SetAxisValue - Action: %s, Value: %.2f, Duration: %.2f"),
		*ActionName.ToString(), Value, Duration);

	return false;
}

APawn* UAutoDriverComponent::GetControlledPawn() const
{
	if (CachedPlayerController)
	{
		return CachedPlayerController->GetPawn();
	}

	// Fallback: if owner is a pawn
	return Cast<APawn>(GetOwner());
}

ACharacter* UAutoDriverComponent::GetControlledCharacter() const
{
	return Cast<ACharacter>(GetControlledPawn());
}

void UAutoDriverComponent::SetEnabled(bool bInEnabled)
{
	if (bEnabled == bInEnabled)
	{
		return;
	}

	bEnabled = bInEnabled;

	if (!bEnabled)
	{
		StopCurrentCommand();
	}

	UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAutoDriverComponent::OnCommandCompleted(const FAutoDriverCommandResult& Result)
{
	// Broadcast completion event
	OnCommandComplete.Broadcast(Result.IsSuccess(), Result.Message);

	// Clear current command
	CurrentCommand.Reset();

	// Log result
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: Command completed successfully - %s"), *Result.Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Command failed - %s"), *Result.Message);
	}
}

AAIController* UAutoDriverComponent::GetOrCreateAIController()
{
	if (!bUseAIControllerForNavigation)
	{
		return nullptr;
	}

	// Return cached controller if valid
	if (CachedAIController && IsValid(CachedAIController))
	{
		return CachedAIController;
	}

	APawn* ControlledPawn = GetControlledPawn();
	if (!ControlledPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot create AI controller - no pawn"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Check if pawn already has an AI controller
	if (AAIController* ExistingAI = Cast<AAIController>(ControlledPawn->GetController()))
	{
		CachedAIController = ExistingAI;
		return ExistingAI;
	}

	// Create new AI controller
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ControlledPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CachedAIController = World->SpawnActor<AAIController>(AAIController::StaticClass(), SpawnParams);
	if (CachedAIController)
	{
		// Possess the pawn with AI controller
		CachedAIController->Possess(ControlledPawn);

		UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: Created AI controller for navigation"));
	}

	return CachedAIController;
}

void UAutoDriverComponent::ReleaseAIController()
{
	if (CachedAIController && IsValid(CachedAIController))
	{
		APawn* ControlledPawn = GetControlledPawn();
		if (ControlledPawn && CachedPlayerController)
		{
			// Re-possess with original player controller
			CachedPlayerController->Possess(ControlledPawn);
		}

		// Destroy AI controller
		CachedAIController->Destroy();
		CachedAIController = nullptr;

		UE_LOG(LogTemp, Log, TEXT("AutoDriverComponent: Released AI controller"));
	}
}

bool UAutoDriverComponent::IsLocationReachable(FVector TargetLocation)
{
	APawn* Pawn = GetControlledPawn();
	if (!Pawn)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return false;
	}

	FPathFindingQuery Query;
	Query.StartLocation = Pawn->GetActorLocation();
	Query.EndLocation = TargetLocation;
	Query.NavData = NavSys->GetDefaultNavDataInstance();

	if (!Query.NavData.IsValid())
	{
		return false;
	}

	FPathFindingResult Result = NavSys->FindPathSync(Query);
	return Result.IsSuccessful() && Result.Path.IsValid();
}

float UAutoDriverComponent::GetPathLengthToLocation(FVector TargetLocation)
{
	APawn* Pawn = GetControlledPawn();
	if (!Pawn)
	{
		return -1.0f;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return -1.0f;
	}

	FPathFindingQuery Query;
	Query.StartLocation = Pawn->GetActorLocation();
	Query.EndLocation = TargetLocation;
	Query.NavData = NavSys->GetDefaultNavDataInstance();

	if (!Query.NavData.IsValid())
	{
		return -1.0f;
	}

	FPathFindingResult Result = NavSys->FindPathSync(Query);
	if (Result.IsSuccessful() && Result.Path.IsValid())
	{
		return Result.Path->GetLength();
	}

	return -1.0f;
}

bool UAutoDriverComponent::GetRandomReachableLocation(float Radius, FVector& OutLocation)
{
	APawn* Pawn = GetControlledPawn();
	if (!Pawn)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return false;
	}

	FNavLocation NavLocation;
	if (NavSys->GetRandomReachablePointInRadius(Pawn->GetActorLocation(), Radius, NavLocation))
	{
		OutLocation = NavLocation.Location;
		return true;
	}

	return false;
}

// ========================================
// UI Methods
// ========================================

bool UAutoDriverComponent::ClickWidget(const FString& WidgetName, const FUIClickParams& ClickParams)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot click widget - component is disabled"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return UUIInteractionHelper::ClickWidgetByName(World, WidgetName, ClickParams);
}

bool UAutoDriverComponent::ClickWidgetByQuery(const FWidgetQueryParams& QueryParams, const FUIClickParams& ClickParams)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot click widget - component is disabled"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Find the widget
	FWidgetInfo WidgetInfo = UWidgetQueryHelper::FindWidget(World, QueryParams);
	if (!WidgetInfo.IsValid())
	{
		return false;
	}

	// Find the actual widget pointer
	UWidget* Widget = UWidgetQueryHelper::FindWidgetByPredicate(World, [QueryParams](UWidget* W)
	{
		if (!W)
		{
			return false;
		}

		switch (QueryParams.QueryType)
		{
			case EWidgetQueryType::ByName:
				return W->GetName().Equals(QueryParams.Name);
			case EWidgetQueryType::ByClass:
				return W->GetClass()->GetName().Contains(QueryParams.ClassName);
			case EWidgetQueryType::ByText:
			{
				FString WidgetText = UWidgetQueryHelper::GetWidgetText(W);
				return WidgetText.Contains(QueryParams.Text);
			}
			default:
				return false;
		}
	});

	if (!Widget)
	{
		return false;
	}

	return UUIInteractionHelper::ClickWidget(World, Widget, ClickParams);
}

bool UAutoDriverComponent::WaitForWidget(const FString& WidgetName, float Timeout)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot wait for widget - component is disabled"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Simple polling implementation
	float ElapsedTime = 0.0f;
	const float PollInterval = 0.1f;

	while (ElapsedTime < Timeout)
	{
		FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, WidgetName);
		if (Info.IsValid())
		{
			return true;
		}

		FPlatformProcess::Sleep(PollInterval);
		ElapsedTime += PollInterval;
	}

	return false;
}

bool UAutoDriverComponent::WaitForWidgetToDisappear(const FString& WidgetName, float Timeout)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverComponent: Cannot wait for widget - component is disabled"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Simple polling implementation
	float ElapsedTime = 0.0f;
	const float PollInterval = 0.1f;

	while (ElapsedTime < Timeout)
	{
		FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, WidgetName);
		if (!Info.IsValid())
		{
			return true;
		}

		FPlatformProcess::Sleep(PollInterval);
		ElapsedTime += PollInterval;
	}

	return false;
}

FWidgetInfo UAutoDriverComponent::FindWidget(const FString& WidgetName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FWidgetInfo();
	}

	return UWidgetQueryHelper::FindWidgetByName(World, WidgetName);
}

TArray<FWidgetInfo> UAutoDriverComponent::FindWidgets(const FWidgetQueryParams& QueryParams)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return TArray<FWidgetInfo>();
	}

	return UWidgetQueryHelper::FindWidgets(World, QueryParams);
}

FString UAutoDriverComponent::GetWidgetText(const FString& WidgetName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FString();
	}

	FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, WidgetName);
	return Info.TextContent;
}

bool UAutoDriverComponent::IsWidgetVisible(const FString& WidgetName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, WidgetName);
	return Info.IsValid() && Info.bIsVisible;
}

TArray<FWidgetInfo> UAutoDriverComponent::GetAllButtons()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return TArray<FWidgetInfo>();
	}

	return UWidgetQueryHelper::FindAllButtons(World);
}
