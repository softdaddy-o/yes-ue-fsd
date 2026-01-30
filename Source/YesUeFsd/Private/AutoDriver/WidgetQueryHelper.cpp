// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/WidgetQueryHelper.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/TextWidgetTypes.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"

FWidgetInfo UWidgetQueryHelper::FindWidget(UWorld* World, const FWidgetQueryParams& QueryParams)
{
	TArray<FWidgetInfo> Results = FindWidgets(World, QueryParams);
	return Results.Num() > 0 ? Results[0] : FWidgetInfo();
}

TArray<FWidgetInfo> UWidgetQueryHelper::FindWidgets(UWorld* World, const FWidgetQueryParams& QueryParams)
{
	TArray<FWidgetInfo> Results;

	if (!World)
	{
		return Results;
	}

	TArray<UUserWidget*> UserWidgets = GetAllActiveUserWidgets(World);

	for (UUserWidget* UserWidget : UserWidgets)
	{
		if (!UserWidget)
		{
			continue;
		}

		TArray<UWidget*> AllWidgets;
		GetWidgetsFromUserWidget(UserWidget, AllWidgets, QueryParams.bRecursive);

		for (UWidget* Widget : AllWidgets)
		{
			if (!Widget)
			{
				continue;
			}

			// Check visibility filter
			if (QueryParams.bVisibleOnly && !IsWidgetVisible(Widget))
			{
				continue;
			}

			// Check if widget matches query
			if (MatchesQuery(Widget, QueryParams))
			{
				FWidgetInfo Info = GetWidgetInfo(Widget);
				Results.Add(Info);

				// Check max results limit
				if (QueryParams.MaxResults > 0 && Results.Num() >= QueryParams.MaxResults)
				{
					return Results;
				}
			}
		}
	}

	return Results;
}

FWidgetInfo UWidgetQueryHelper::FindWidgetByName(UWorld* World, const FString& WidgetName)
{
	return FindWidget(World, FWidgetQueryParams::ByWidgetName(WidgetName));
}

TArray<FWidgetInfo> UWidgetQueryHelper::FindWidgetsByText(UWorld* World, const FString& SearchText, bool bExactMatch)
{
	FWidgetQueryParams QueryParams = FWidgetQueryParams::ByWidgetText(SearchText);
	QueryParams.bCaseSensitive = bExactMatch;
	return FindWidgets(World, QueryParams);
}

TArray<FWidgetInfo> UWidgetQueryHelper::FindAllButtons(UWorld* World)
{
	return FindWidgets(World, FWidgetQueryParams::ByWidgetClass("Button"));
}

FWidgetInfo UWidgetQueryHelper::GetWidgetInfo(UWidget* Widget)
{
	FWidgetInfo Info;

	if (!Widget)
	{
		return Info;
	}

	Info.bFound = true;
	Info.Name = Widget->GetName();
	Info.ClassName = Widget->GetClass()->GetName();
	Info.Path = GetWidgetPath(Widget);
	Info.bIsVisible = IsWidgetVisible(Widget);
	Info.bIsEnabled = IsWidgetEnabled(Widget);
	Info.TextContent = GetWidgetText(Widget);

	// Get geometry
	GetWidgetGeometry(Widget, Info.Position, Info.Size);

	return Info;
}

FString UWidgetQueryHelper::GetWidgetText(UWidget* Widget)
{
	if (!Widget)
	{
		return FString();
	}

	// Check for TextBlock
	if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		return TextBlock->GetText().ToString();
	}

	// Check for Button (has text child)
	if (UButton* Button = Cast<UButton>(Widget))
	{
		// Try to find text child
		TArray<UWidget*> Children = GetAllChildWidgets(Button, false);
		for (UWidget* Child : Children)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Child))
			{
				return TextBlock->GetText().ToString();
			}
		}
	}

	// Check for EditableText
	if (UEditableText* EditableText = Cast<UEditableText>(Widget))
	{
		return EditableText->GetText().ToString();
	}

	// Check for EditableTextBox
	if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
	{
		return EditableTextBox->GetText().ToString();
	}

	return FString();
}

bool UWidgetQueryHelper::GetWidgetGeometry(UWidget* Widget, FVector2D& OutPosition, FVector2D& OutSize)
{
	if (!Widget)
	{
		return false;
	}

	const FGeometry& Geometry = Widget->GetCachedGeometry();

	if (!Geometry.IsValid())
	{
		return false;
	}

	// Get absolute position and size
	OutPosition = Geometry.GetAbsolutePosition();
	OutSize = Geometry.GetAbsoluteSize();

	return true;
}

bool UWidgetQueryHelper::IsWidgetVisible(UWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	// Check widget visibility
	if (Widget->GetVisibility() == ESlateVisibility::Hidden ||
		Widget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return false;
	}

	// Check parent visibility recursively
	UWidget* Parent = Widget->GetParent();
	if (Parent)
	{
		return IsWidgetVisible(Parent);
	}

	return true;
}

bool UWidgetQueryHelper::IsWidgetEnabled(UWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	return Widget->GetIsEnabled();
}

FString UWidgetQueryHelper::GetWidgetPath(UWidget* Widget)
{
	if (!Widget)
	{
		return FString();
	}

	TArray<FString> PathComponents;
	UWidget* Current = Widget;

	while (Current)
	{
		PathComponents.Insert(Current->GetName(), 0);
		Current = Current->GetParent();
	}

	return FString::Join(PathComponents, TEXT("/"));
}

TArray<UUserWidget*> UWidgetQueryHelper::GetAllActiveUserWidgets(UWorld* World)
{
	TArray<UUserWidget*> UserWidgets;

	if (!World)
	{
		return UserWidgets;
	}

	// Get all UUserWidget objects in the world
	for (TObjectIterator<UUserWidget> It; It; ++It)
	{
		UUserWidget* Widget = *It;

		// Check if widget belongs to this world
		if (Widget && Widget->GetWorld() == World)
		{
			// Check if widget is in viewport
			if (Widget->IsInViewport())
			{
				UserWidgets.Add(Widget);
			}
		}
	}

	return UserWidgets;
}

TArray<UWidget*> UWidgetQueryHelper::GetAllChildWidgets(UWidget* ParentWidget, bool bRecursive)
{
	TArray<UWidget*> Widgets;

	if (!ParentWidget)
	{
		return Widgets;
	}

	TraverseWidgetTree(ParentWidget, [&Widgets](UWidget* Widget)
	{
		if (Widget)
		{
			Widgets.Add(Widget);
		}
	}, bRecursive);

	return Widgets;
}

UWidget* UWidgetQueryHelper::FindWidgetByPredicate(UWorld* World, TFunction<bool(UWidget*)> Predicate)
{
	if (!World || !Predicate)
	{
		return nullptr;
	}

	TArray<UUserWidget*> UserWidgets = GetAllActiveUserWidgets(World);

	for (UUserWidget* UserWidget : UserWidgets)
	{
		if (!UserWidget)
		{
			continue;
		}

		TArray<UWidget*> AllWidgets;
		GetWidgetsFromUserWidget(UserWidget, AllWidgets, true);

		for (UWidget* Widget : AllWidgets)
		{
			if (Widget && Predicate(Widget))
			{
				return Widget;
			}
		}
	}

	return nullptr;
}

TArray<UWidget*> UWidgetQueryHelper::FindAllWidgetsByPredicate(UWorld* World, TFunction<bool(UWidget*)> Predicate)
{
	TArray<UWidget*> Results;

	if (!World || !Predicate)
	{
		return Results;
	}

	TArray<UUserWidget*> UserWidgets = GetAllActiveUserWidgets(World);

	for (UUserWidget* UserWidget : UserWidgets)
	{
		if (!UserWidget)
		{
			continue;
		}

		TArray<UWidget*> AllWidgets;
		GetWidgetsFromUserWidget(UserWidget, AllWidgets, true);

		for (UWidget* Widget : AllWidgets)
		{
			if (Widget && Predicate(Widget))
			{
				Results.Add(Widget);
			}
		}
	}

	return Results;
}

bool UWidgetQueryHelper::MatchesQuery(UWidget* Widget, const FWidgetQueryParams& QueryParams)
{
	if (!Widget)
	{
		return false;
	}

	switch (QueryParams.QueryType)
	{
		case EWidgetQueryType::ByName:
		{
			FString WidgetName = Widget->GetName();
			FString SearchName = QueryParams.Name;

			if (!QueryParams.bCaseSensitive)
			{
				WidgetName = WidgetName.ToLower();
				SearchName = SearchName.ToLower();
			}

			return WidgetName.Equals(SearchName);
		}

		case EWidgetQueryType::ByClass:
		{
			FString ClassName = Widget->GetClass()->GetName();
			FString SearchClass = QueryParams.ClassName;

			if (!QueryParams.bCaseSensitive)
			{
				ClassName = ClassName.ToLower();
				SearchClass = SearchClass.ToLower();
			}

			return ClassName.Contains(SearchClass);
		}

		case EWidgetQueryType::ByText:
		{
			FString WidgetText = GetWidgetText(Widget);
			FString SearchText = QueryParams.Text;

			if (WidgetText.IsEmpty())
			{
				return false;
			}

			if (!QueryParams.bCaseSensitive)
			{
				WidgetText = WidgetText.ToLower();
				SearchText = SearchText.ToLower();
			}

			return WidgetText.Contains(SearchText);
		}

		default:
			return false;
	}
}

void UWidgetQueryHelper::TraverseWidgetTree(UWidget* Widget, TFunction<void(UWidget*)> Callback, bool bRecursive)
{
	if (!Widget || !Callback)
	{
		return;
	}

	// Process this widget
	Callback(Widget);

	// Process children if recursive
	if (bRecursive)
	{
		// Handle Panel widgets (containers)
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
		{
			for (int32 i = 0; i < PanelWidget->GetChildrenCount(); ++i)
			{
				UWidget* Child = PanelWidget->GetChildAt(i);
				if (Child)
				{
					TraverseWidgetTree(Child, Callback, bRecursive);
				}
			}
		}

		// Handle UserWidget (has WidgetTree)
		if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (UserWidget->WidgetTree)
			{
				UWidget* RootWidget = UserWidget->WidgetTree->RootWidget;
				if (RootWidget && RootWidget != Widget)
				{
					TraverseWidgetTree(RootWidget, Callback, bRecursive);
				}
			}
		}
	}
}

void UWidgetQueryHelper::GetWidgetsFromUserWidget(UUserWidget* UserWidget, TArray<UWidget*>& OutWidgets, bool bRecursive)
{
	if (!UserWidget || !UserWidget->WidgetTree)
	{
		return;
	}

	// Get all widgets from the WidgetTree
	TArray<UWidget*> AllWidgets;
	UserWidget->WidgetTree->GetAllWidgets(AllWidgets);

	for (UWidget* Widget : AllWidgets)
	{
		if (Widget)
		{
			OutWidgets.Add(Widget);
		}
	}
}
