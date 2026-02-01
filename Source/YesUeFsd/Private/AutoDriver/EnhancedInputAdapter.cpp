// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/EnhancedInputAdapter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UEnhancedInputAdapter::Initialize(APlayerController* InPlayerController)
{
	if (!InPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputAdapter: Cannot initialize with null PlayerController"));
		return;
	}

	PlayerController = InPlayerController;

	// Try to get Enhanced Input Component
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: PlayerController does not have UEnhancedInputComponent. Enhanced Input features may not work."));
	}

	// Get Enhanced Input Subsystem
	InputSubsystem = GetInputSubsystem();
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Could not get Enhanced Input Subsystem"));
	}

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Initialized for PlayerController: %s"), *PlayerController->GetName());
}

void UEnhancedInputAdapter::RegisterActionMapping(const FEnhancedInputActionMapping& Mapping)
{
	if (!Mapping.InputAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Cannot register mapping with null InputAction"));
		return;
	}

	// Check if already registered
	for (const FEnhancedInputActionMapping& ExistingMapping : ActionMappings)
	{
		if (ExistingMapping.ActionName == Mapping.ActionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Action %s is already registered. Updating mapping."), *Mapping.ActionName.ToString());
			// Remove old mapping
			ActionMappings.Remove(ExistingMapping);
			break;
		}
	}

	ActionMappings.Add(Mapping);

	// If mapping has a context, add it
	if (Mapping.MappingContext)
	{
		AddMappingContext(Mapping.MappingContext, Mapping.Priority);
	}

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Registered action mapping: %s -> %s"),
		*Mapping.ActionName.ToString(),
		*Mapping.InputAction->GetName());
}

void UEnhancedInputAdapter::RegisterActionMappings(const TArray<FEnhancedInputActionMapping>& Mappings)
{
	for (const FEnhancedInputActionMapping& Mapping : Mappings)
	{
		RegisterActionMapping(Mapping);
	}
}

UInputAction* UEnhancedInputAdapter::FindInputAction(FName ActionName) const
{
	for (const FEnhancedInputActionMapping& Mapping : ActionMappings)
	{
		if (Mapping.ActionName == ActionName)
		{
			return Mapping.InputAction;
		}
	}

	return nullptr;
}

void UEnhancedInputAdapter::AddMappingContext(UInputMappingContext* MappingContext, int32 Priority)
{
	if (!MappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Cannot add null MappingContext"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputAdapter: Cannot add mapping context - no input subsystem"));
		return;
	}

	// Check if already active
	if (ActiveContexts.Contains(MappingContext))
	{
		UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Mapping context %s is already active. Updating priority."), *MappingContext->GetName());
		Subsystem->RemoveMappingContext(MappingContext);
	}

	Subsystem->AddMappingContext(MappingContext, Priority);
	ActiveContexts.Add(MappingContext, Priority);

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Added mapping context: %s (Priority: %d)"),
		*MappingContext->GetName(), Priority);
}

void UEnhancedInputAdapter::RemoveMappingContext(UInputMappingContext* MappingContext)
{
	if (!MappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
	if (!Subsystem)
	{
		return;
	}

	if (ActiveContexts.Contains(MappingContext))
	{
		Subsystem->RemoveMappingContext(MappingContext);
		ActiveContexts.Remove(MappingContext);

		UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Removed mapping context: %s"), *MappingContext->GetName());
	}
}

void UEnhancedInputAdapter::ClearAllMappingContexts()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
	if (!Subsystem)
	{
		return;
	}

	for (const TPair<TObjectPtr<UInputMappingContext>, int32>& Pair : ActiveContexts)
	{
		if (Pair.Key)
		{
			Subsystem->RemoveMappingContext(Pair.Key);
		}
	}

	ActiveContexts.Empty();
	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Cleared all mapping contexts"));
}

TArray<UInputMappingContext*> UEnhancedInputAdapter::GetActiveMappingContexts() const
{
	// UE 5.7: GetKeys requires matching template types
	TArray<TObjectPtr<UInputMappingContext>> ContextKeys;
	ActiveContexts.GetKeys(ContextKeys);

	TArray<UInputMappingContext*> Contexts;
	for (const TObjectPtr<UInputMappingContext>& Context : ContextKeys)
	{
		Contexts.Add(Context.Get());
	}
	return Contexts;
}

bool UEnhancedInputAdapter::InjectInputAction(FName ActionName, FInputActionValue ActionValue)
{
	UInputAction* InputAction = FindInputAction(ActionName);
	if (!InputAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Could not find InputAction for %s"), *ActionName.ToString());
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputAdapter: Cannot inject input - no input subsystem"));
		return false;
	}

	// Inject the input (UE 5.7: InjectInputForAction now requires Modifiers and Triggers arrays)
	TArray<UInputModifier*> Modifiers;
	TArray<UInputTrigger*> Triggers;
	Subsystem->InjectInputForAction(InputAction, ActionValue, Modifiers, Triggers);

	UE_LOG(LogTemp, Verbose, TEXT("EnhancedInputAdapter: Injected input for %s"), *ActionName.ToString());
	return true;
}

bool UEnhancedInputAdapter::InjectButtonPress(FName ActionName)
{
	return InjectInputAction(ActionName, FInputActionValue(true));
}

bool UEnhancedInputAdapter::InjectButtonRelease(FName ActionName)
{
	return InjectInputAction(ActionName, FInputActionValue(false));
}

bool UEnhancedInputAdapter::InjectAxisValue(FName ActionName, float Value)
{
	return InjectInputAction(ActionName, FInputActionValue(Value));
}

bool UEnhancedInputAdapter::InjectAxis2DValue(FName ActionName, FVector2D Value)
{
	return InjectInputAction(ActionName, FInputActionValue(Value));
}

bool UEnhancedInputAdapter::InjectAxis3DValue(FName ActionName, FVector Value)
{
	return InjectInputAction(ActionName, FInputActionValue(Value));
}

void UEnhancedInputAdapter::StartRecording()
{
	if (bIsRecording)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Already recording"));
		return;
	}

	RecordedActions.Empty();
	RecordingStartTime = PlayerController ? PlayerController->GetWorld()->GetTimeSeconds() : 0.0f;
	bIsRecording = true;

	SetupRecordingBindings();

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Started recording"));
}

void UEnhancedInputAdapter::StopRecording()
{
	if (!bIsRecording)
	{
		return;
	}

	bIsRecording = false;
	ClearRecordingBindings();

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Stopped recording. Recorded %d actions"), RecordedActions.Num());
}

void UEnhancedInputAdapter::ClearRecordedActions()
{
	RecordedActions.Empty();
	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Cleared recorded actions"));
}

FString UEnhancedInputAdapter::ExportRecordingToJSON() const
{
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ActionsArray;

	for (const FInputActionRecord& Record : RecordedActions)
	{
		TSharedPtr<FJsonObject> ActionObject = MakeShared<FJsonObject>();

		ActionObject->SetNumberField(TEXT("Timestamp"), Record.Timestamp);
		ActionObject->SetStringField(TEXT("ActionName"), Record.ActionName.ToString());
		ActionObject->SetBoolField(TEXT("Triggered"), Record.bTriggered);
		ActionObject->SetBoolField(TEXT("Started"), Record.bStarted);
		ActionObject->SetBoolField(TEXT("Completed"), Record.bCompleted);

		// Serialize InputActionValue
		FString ValueTypeString;
		switch (Record.ActionValue.GetValueType())
		{
		case EInputActionValueType::Boolean:
			ValueTypeString = TEXT("Boolean");
			ActionObject->SetBoolField(TEXT("Value"), Record.ActionValue.Get<bool>());
			break;
		case EInputActionValueType::Axis1D:
			ValueTypeString = TEXT("Axis1D");
			ActionObject->SetNumberField(TEXT("Value"), Record.ActionValue.Get<float>());
			break;
		case EInputActionValueType::Axis2D:
			{
				ValueTypeString = TEXT("Axis2D");
				FVector2D Value = Record.ActionValue.Get<FVector2D>();
				TSharedPtr<FJsonObject> ValueObject = MakeShared<FJsonObject>();
				ValueObject->SetNumberField(TEXT("X"), Value.X);
				ValueObject->SetNumberField(TEXT("Y"), Value.Y);
				ActionObject->SetObjectField(TEXT("Value"), ValueObject);
			}
			break;
		case EInputActionValueType::Axis3D:
			{
				ValueTypeString = TEXT("Axis3D");
				FVector Value = Record.ActionValue.Get<FVector>();
				TSharedPtr<FJsonObject> ValueObject = MakeShared<FJsonObject>();
				ValueObject->SetNumberField(TEXT("X"), Value.X);
				ValueObject->SetNumberField(TEXT("Y"), Value.Y);
				ValueObject->SetNumberField(TEXT("Z"), Value.Z);
				ActionObject->SetObjectField(TEXT("Value"), ValueObject);
			}
			break;
		}
		ActionObject->SetStringField(TEXT("ValueType"), ValueTypeString);

		if (Record.InputAction)
		{
			ActionObject->SetStringField(TEXT("InputActionPath"), Record.InputAction->GetPathName());
		}

		if (Record.ActiveContext)
		{
			ActionObject->SetStringField(TEXT("ContextPath"), Record.ActiveContext->GetPathName());
		}

		ActionsArray.Add(MakeShared<FJsonValueObject>(ActionObject));
	}

	RootObject->SetArrayField(TEXT("RecordedActions"), ActionsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

bool UEnhancedInputAdapter::ImportRecordingFromJSON(const FString& JSONString)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputAdapter: Failed to parse JSON"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* ActionsArray;
	if (!RootObject->TryGetArrayField(TEXT("RecordedActions"), ActionsArray))
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputAdapter: No RecordedActions array in JSON"));
		return false;
	}

	RecordedActions.Empty();

	for (const TSharedPtr<FJsonValue>& ActionValue : *ActionsArray)
	{
		const TSharedPtr<FJsonObject>* ActionObject;
		if (!ActionValue->TryGetObject(ActionObject))
		{
			continue;
		}

		FInputActionRecord Record;

		(*ActionObject)->TryGetNumberField(TEXT("Timestamp"), Record.Timestamp);

		FString ActionNameString;
		if ((*ActionObject)->TryGetStringField(TEXT("ActionName"), ActionNameString))
		{
			Record.ActionName = FName(*ActionNameString);
		}

		(*ActionObject)->TryGetBoolField(TEXT("Triggered"), Record.bTriggered);
		(*ActionObject)->TryGetBoolField(TEXT("Started"), Record.bStarted);
		(*ActionObject)->TryGetBoolField(TEXT("Completed"), Record.bCompleted);

		// Deserialize InputActionValue
		FString ValueType;
		if ((*ActionObject)->TryGetStringField(TEXT("ValueType"), ValueType))
		{
			if (ValueType == TEXT("Boolean"))
			{
				bool BoolValue = false;
				(*ActionObject)->TryGetBoolField(TEXT("Value"), BoolValue);
				Record.ActionValue = FInputActionValue(BoolValue);
			}
			else if (ValueType == TEXT("Axis1D"))
			{
				double FloatValue = 0.0;
				(*ActionObject)->TryGetNumberField(TEXT("Value"), FloatValue);
				Record.ActionValue = FInputActionValue(static_cast<float>(FloatValue));
			}
			else if (ValueType == TEXT("Axis2D"))
			{
				const TSharedPtr<FJsonObject>* ValueObject;
				if ((*ActionObject)->TryGetObjectField(TEXT("Value"), ValueObject))
				{
					double X = 0.0, Y = 0.0;
					(*ValueObject)->TryGetNumberField(TEXT("X"), X);
					(*ValueObject)->TryGetNumberField(TEXT("Y"), Y);
					Record.ActionValue = FInputActionValue(FVector2D(X, Y));
				}
			}
			else if (ValueType == TEXT("Axis3D"))
			{
				const TSharedPtr<FJsonObject>* ValueObject;
				if ((*ActionObject)->TryGetObjectField(TEXT("Value"), ValueObject))
				{
					double X = 0.0, Y = 0.0, Z = 0.0;
					(*ValueObject)->TryGetNumberField(TEXT("X"), X);
					(*ValueObject)->TryGetNumberField(TEXT("Y"), Y);
					(*ValueObject)->TryGetNumberField(TEXT("Z"), Z);
					Record.ActionValue = FInputActionValue(FVector(X, Y, Z));
				}
			}
		}

		// Note: InputAction and ActiveContext paths are stored but not loaded
		// as they require asset loading which should be done separately

		RecordedActions.Add(Record);
	}

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Imported %d recorded actions from JSON"), RecordedActions.Num());
	return true;
}

UEnhancedInputAdapter* UEnhancedInputAdapter::CreateEnhancedInputAdapter(UObject* WorldContextObject, APlayerController* InPlayerController)
{
	if (!WorldContextObject || !InPlayerController)
	{
		return nullptr;
	}

	UEnhancedInputAdapter* Adapter = NewObject<UEnhancedInputAdapter>();
	Adapter->Initialize(InPlayerController);

	return Adapter;
}

UEnhancedInputComponent* UEnhancedInputAdapter::GetEnhancedInputComponent()
{
	if (!EnhancedInputComponent && PlayerController)
	{
		EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	}

	return EnhancedInputComponent;
}

UEnhancedInputLocalPlayerSubsystem* UEnhancedInputAdapter::GetInputSubsystem()
{
	if (!InputSubsystem && PlayerController)
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}

	return InputSubsystem;
}

void UEnhancedInputAdapter::RecordInputAction(UInputAction* InputAction, FName ActionName, const FInputActionValue& Value, bool bTriggered, bool bStarted, bool bCompleted)
{
	if (!bIsRecording)
	{
		return;
	}

	FInputActionRecord Record;
	Record.Timestamp = PlayerController ? PlayerController->GetWorld()->GetTimeSeconds() - RecordingStartTime : 0.0f;
	Record.InputAction = InputAction;
	Record.ActionName = ActionName;
	Record.ActionValue = Value;
	Record.bTriggered = bTriggered;
	Record.bStarted = bStarted;
	Record.bCompleted = bCompleted;

	// Try to find active context for this action
	for (const FEnhancedInputActionMapping& Mapping : ActionMappings)
	{
		if (Mapping.InputAction == InputAction && Mapping.MappingContext)
		{
			Record.ActiveContext = Mapping.MappingContext;
			break;
		}
	}

	RecordedActions.Add(Record);
}

void UEnhancedInputAdapter::SetupRecordingBindings()
{
	UEnhancedInputComponent* Component = GetEnhancedInputComponent();
	if (!Component)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputAdapter: Cannot setup recording - no Enhanced Input Component"));
		return;
	}

	// Bind to all registered actions
	for (const FEnhancedInputActionMapping& Mapping : ActionMappings)
	{
		if (!Mapping.InputAction)
		{
			continue;
		}

		// Bind to all trigger states for complete recording
		FName ActionName = Mapping.ActionName;
		UInputAction* InputAction = Mapping.InputAction;

		// Triggered callback - UE 5.7 API: Use BindActionValueLambda for lambda callbacks
		FEnhancedInputActionEventBinding& TriggeredBinding = Component->BindActionValueLambda(InputAction, ETriggerEvent::Triggered,
			[this, InputAction, ActionName](const FInputActionValue& Value)
			{
				RecordInputAction(InputAction, ActionName, Value, true, false, false);
			});
		RecordingBindingHandles.Add(TriggeredBinding.GetHandle());

		// Started callback
		FEnhancedInputActionEventBinding& StartedBinding = Component->BindActionValueLambda(InputAction, ETriggerEvent::Started,
			[this, InputAction, ActionName](const FInputActionValue& Value)
			{
				RecordInputAction(InputAction, ActionName, Value, false, true, false);
			});
		RecordingBindingHandles.Add(StartedBinding.GetHandle());

		// Completed callback
		FEnhancedInputActionEventBinding& CompletedBinding = Component->BindActionValueLambda(InputAction, ETriggerEvent::Completed,
			[this, InputAction, ActionName](const FInputActionValue& Value)
			{
				RecordInputAction(InputAction, ActionName, Value, false, false, true);
			});
		RecordingBindingHandles.Add(CompletedBinding.GetHandle());
	}

	UE_LOG(LogTemp, Log, TEXT("EnhancedInputAdapter: Setup recording bindings for %d actions"), ActionMappings.Num());
}

void UEnhancedInputAdapter::ClearRecordingBindings()
{
	UEnhancedInputComponent* Component = GetEnhancedInputComponent();
	if (!Component)
	{
		return;
	}

	// Remove all bindings using stored handles (UE 5.7: GetActionEventBinding removed)
	for (const int32& Handle : RecordingBindingHandles)
	{
		Component->RemoveBindingByHandle(Handle);
	}

	RecordingBindingHandles.Empty();
}
