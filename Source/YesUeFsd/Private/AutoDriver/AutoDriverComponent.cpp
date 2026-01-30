// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "AIController.h"

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
	// TODO: Create or get AI controller for navigation-based movement
	return nullptr;
}
