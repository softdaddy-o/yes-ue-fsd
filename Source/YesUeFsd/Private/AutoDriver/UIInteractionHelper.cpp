// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/UIInteractionHelper.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"

bool UUIInteractionHelper::ClickWidget(UWorld* World, UWidget* Widget, const FUIClickParams& ClickParams)
{
	if (!World || !Widget)
	{
		return false;
	}

	// Get widget center position
	FVector2D ClickPosition;
	if (!GetWidgetCenterPosition(Widget, ClickPosition))
	{
		return false;
	}

	// Apply offset from center
	FVector2D WidgetSize;
	FVector2D WidgetPos;
	if (UWidgetQueryHelper::GetWidgetGeometry(Widget, WidgetPos, WidgetSize))
	{
		ClickPosition += FVector2D(
			ClickParams.OffsetFromCenter.X * WidgetSize.X,
			ClickParams.OffsetFromCenter.Y * WidgetSize.Y
		);
	}

	// Perform clicks
	for (int32 i = 0; i < ClickParams.ClickCount; ++i)
	{
		// Mouse down
		if (!SimulateMouseButtonEvent(World, ClickPosition, ClickParams.ClickType, true))
		{
			return false;
		}

		// Small delay
		FPlatformProcess::Sleep(0.01f);

		// Mouse up
		if (!SimulateMouseButtonEvent(World, ClickPosition, ClickParams.ClickType, false))
		{
			return false;
		}

		// Delay between clicks
		if (i < ClickParams.ClickCount - 1)
		{
			FPlatformProcess::Sleep(ClickParams.ClickDelay);
		}
	}

	return true;
}

bool UUIInteractionHelper::ClickWidgetByName(UWorld* World, const FString& WidgetName, const FUIClickParams& ClickParams)
{
	if (!World)
	{
		return false;
	}

	FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, WidgetName);

	if (!Info.IsValid())
	{
		return false;
	}

	// Find the actual widget pointer
	UWidget* Widget = UWidgetQueryHelper::FindWidgetByPredicate(World, [WidgetName](UWidget* W)
	{
		return W && W->GetName().Equals(WidgetName);
	});

	if (!Widget)
	{
		return false;
	}

	return ClickWidget(World, Widget, ClickParams);
}

bool UUIInteractionHelper::ClickAtScreenPosition(UWorld* World, const FVector2D& ScreenPosition, const FUIClickParams& ClickParams)
{
	if (!World)
	{
		return false;
	}

	// Perform clicks
	for (int32 i = 0; i < ClickParams.ClickCount; ++i)
	{
		// Mouse down
		if (!SimulateMouseButtonEvent(World, ScreenPosition, ClickParams.ClickType, true))
		{
			return false;
		}

		// Small delay
		FPlatformProcess::Sleep(0.01f);

		// Mouse up
		if (!SimulateMouseButtonEvent(World, ScreenPosition, ClickParams.ClickType, false))
		{
			return false;
		}

		// Delay between clicks
		if (i < ClickParams.ClickCount - 1)
		{
			FPlatformProcess::Sleep(ClickParams.ClickDelay);
		}
	}

	return true;
}

bool UUIInteractionHelper::HoverWidget(UWorld* World, UWidget* Widget)
{
	if (!World || !Widget)
	{
		return false;
	}

	FVector2D HoverPosition;
	if (!GetWidgetCenterPosition(Widget, HoverPosition))
	{
		return false;
	}

	return MoveMouseToPosition(World, HoverPosition);
}

bool UUIInteractionHelper::HoverWidgetByName(UWorld* World, const FString& WidgetName)
{
	if (!World)
	{
		return false;
	}

	UWidget* Widget = UWidgetQueryHelper::FindWidgetByPredicate(World, [WidgetName](UWidget* W)
	{
		return W && W->GetName().Equals(WidgetName);
	});

	if (!Widget)
	{
		return false;
	}

	return HoverWidget(World, Widget);
}

bool UUIInteractionHelper::MoveMouseToPosition(UWorld* World, const FVector2D& ScreenPosition)
{
	if (!World)
	{
		return false;
	}

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		SlateApp.SetCursorPos(ScreenPosition);
		return true;
	}

	return false;
}

bool UUIInteractionHelper::SetTextInWidget(UWorld* World, UWidget* Widget, const FString& Text)
{
	if (!World || !Widget)
	{
		return false;
	}

	// Try EditableText
	if (UEditableText* EditableText = Cast<UEditableText>(Widget))
	{
		EditableText->SetText(FText::FromString(Text));
		return true;
	}

	// Try EditableTextBox
	if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
	{
		EditableTextBox->SetText(FText::FromString(Text));
		return true;
	}

	return false;
}

bool UUIInteractionHelper::SetTextInWidgetByName(UWorld* World, const FString& WidgetName, const FString& Text)
{
	if (!World)
	{
		return false;
	}

	UWidget* Widget = UWidgetQueryHelper::FindWidgetByPredicate(World, [WidgetName](UWidget* W)
	{
		return W && W->GetName().Equals(WidgetName);
	});

	if (!Widget)
	{
		return false;
	}

	return SetTextInWidget(World, Widget, Text);
}

bool UUIInteractionHelper::FocusWidget(UWorld* World, UWidget* Widget)
{
	if (!World || !Widget)
	{
		return false;
	}

	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
		if (SlateWidget.IsValid())
		{
			FSlateApplication::Get().SetUserFocus(0, SlateWidget);
			return true;
		}
	}

	return false;
}

bool UUIInteractionHelper::ScrollWidgetIntoView(UWorld* World, UWidget* Widget)
{
	if (!World || !Widget)
	{
		return false;
	}

	// Find parent ScrollBox
	UWidget* Parent = Widget->GetParent();
	while (Parent)
	{
		if (UScrollBox* ScrollBox = Cast<UScrollBox>(Parent))
		{
			ScrollBox->ScrollWidgetIntoView(Widget, true);
			return true;
		}
		Parent = Parent->GetParent();
	}

	return false;
}

bool UUIInteractionHelper::ScrollWidget(UWorld* World, UWidget* Widget, float ScrollDelta)
{
	if (!World || !Widget)
	{
		return false;
	}

	if (UScrollBox* ScrollBox = Cast<UScrollBox>(Widget))
	{
		float CurrentOffset = ScrollBox->GetScrollOffset();
		ScrollBox->SetScrollOffset(CurrentOffset + ScrollDelta);
		return true;
	}

	return false;
}

bool UUIInteractionHelper::SimulateMouseButtonEvent(UWorld* World, const FVector2D& ScreenPosition, EUIClickType ClickType, bool bIsPressed)
{
	if (!World || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	FSlateApplication& SlateApp = FSlateApplication::Get();
	FKey MouseButton = GetMouseButtonKey(ClickType);

	// Create pointer event
	FPointerEvent PointerEvent(
		0, // User index
		FSlateApplication::CursorPointerIndex, // Pointer index
		ScreenPosition,
		ScreenPosition, // Last screen position
		TSet<FKey>(), // Pressed buttons
		MouseButton, // Effecting button
		0.0f, // Wheel delta
		FModifierKeysState() // Modifier keys
	);

	if (bIsPressed)
	{
		return SlateApp.ProcessMouseButtonDownEvent(nullptr, PointerEvent);
	}
	else
	{
		return SlateApp.ProcessMouseButtonUpEvent(PointerEvent);
	}
}

UGameViewportClient* UUIInteractionHelper::GetGameViewportClient(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	return World->GetGameViewport();
}

FKey UUIInteractionHelper::GetMouseButtonKey(EUIClickType ClickType)
{
	switch (ClickType)
	{
		case EUIClickType::Left:
			return EKeys::LeftMouseButton;
		case EUIClickType::Right:
			return EKeys::RightMouseButton;
		case EUIClickType::Middle:
			return EKeys::MiddleMouseButton;
		default:
			return EKeys::LeftMouseButton;
	}
}

bool UUIInteractionHelper::GetWidgetCenterPosition(UWidget* Widget, FVector2D& OutPosition)
{
	if (!Widget)
	{
		return false;
	}

	FVector2D Position, Size;
	if (UWidgetQueryHelper::GetWidgetGeometry(Widget, Position, Size))
	{
		OutPosition = Position + (Size * 0.5f);
		return true;
	}

	return false;
}
