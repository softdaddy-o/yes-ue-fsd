// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriverUITypes.generated.h"

/**
 * Widget query type
 */
UENUM(BlueprintType)
enum class EWidgetQueryType : uint8
{
	/** Find by exact widget name */
	ByName,

	/** Find by widget class */
	ByClass,

	/** Find by text content */
	ByText,

	/** Custom query logic */
	Custom
};

/**
 * UI click type
 */
UENUM(BlueprintType)
enum class EUIClickType : uint8
{
	/** Left mouse button */
	Left,

	/** Right mouse button */
	Right,

	/** Middle mouse button */
	Middle
};

/**
 * Widget query parameters
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FWidgetQueryParams
{
	GENERATED_BODY()

	/** Query type */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	EWidgetQueryType QueryType = EWidgetQueryType::ByName;

	/** Widget name (for ByName queries) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString Name;

	/** Widget class name (for ByClass queries) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString ClassName;

	/** Text content to search for (for ByText queries) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString Text;

	/** Search in child widgets recursively */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bRecursive = true;

	/** Only return visible widgets */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bVisibleOnly = true;

	/** Case-sensitive search */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bCaseSensitive = false;

	/** Maximum number of results to return (0 = unlimited) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	int32 MaxResults = 0;

	FWidgetQueryParams() = default;

	/** Helper constructor for name-based query */
	static FWidgetQueryParams ByWidgetName(const FString& InName)
	{
		FWidgetQueryParams Params;
		Params.QueryType = EWidgetQueryType::ByName;
		Params.Name = InName;
		return Params;
	}

	/** Helper constructor for class-based query */
	static FWidgetQueryParams ByWidgetClass(const FString& InClassName)
	{
		FWidgetQueryParams Params;
		Params.QueryType = EWidgetQueryType::ByClass;
		Params.ClassName = InClassName;
		return Params;
	}

	/** Helper constructor for text-based query */
	static FWidgetQueryParams ByWidgetText(const FString& InText)
	{
		FWidgetQueryParams Params;
		Params.QueryType = EWidgetQueryType::ByText;
		Params.Text = InText;
		return Params;
	}
};

/**
 * Widget information structure
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FWidgetInfo
{
	GENERATED_BODY()

	/** Widget name */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString Name;

	/** Widget class name */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString ClassName;

	/** Full widget path in hierarchy */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString Path;

	/** Screen position (top-left) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FVector2D Position = FVector2D::ZeroVector;

	/** Widget size */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FVector2D Size = FVector2D::ZeroVector;

	/** Is widget visible */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bIsVisible = false;

	/** Is widget enabled/interactive */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bIsEnabled = false;

	/** Text content (if widget has text) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FString TextContent;

	/** Was the widget found */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bFound = false;

	FWidgetInfo() = default;

	/** Check if widget info is valid */
	bool IsValid() const { return bFound && !Name.IsEmpty(); }

	/** Get center position */
	FVector2D GetCenter() const { return Position + (Size * 0.5f); }
};

/**
 * UI click parameters
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FUIClickParams
{
	GENERATED_BODY()

	/** Click type (left, right, middle) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	EUIClickType ClickType = EUIClickType::Left;

	/** Number of clicks (1 = single, 2 = double, etc.) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	int32 ClickCount = 1;

	/** Offset from widget center (normalized 0-1) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FVector2D OffsetFromCenter = FVector2D::ZeroVector;

	/** Delay between clicks for multi-click (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float ClickDelay = 0.1f;

	FUIClickParams() = default;

	/** Helper for left click */
	static FUIClickParams LeftClick()
	{
		return FUIClickParams();
	}

	/** Helper for right click */
	static FUIClickParams RightClick()
	{
		FUIClickParams Params;
		Params.ClickType = EUIClickType::Right;
		return Params;
	}

	/** Helper for double click */
	static FUIClickParams DoubleClick()
	{
		FUIClickParams Params;
		Params.ClickCount = 2;
		return Params;
	}

	/** Convert click type to string */
	static FString ClickTypeToString(EUIClickType InClickType)
	{
		switch (InClickType)
		{
			case EUIClickType::Right:  return TEXT("Right");
			case EUIClickType::Middle: return TEXT("Middle");
			default:                   return TEXT("Left");
		}
	}

	/** Convert string to click type */
	static EUIClickType StringToClickType(const FString& InString)
	{
		if (InString.Equals(TEXT("Right"), ESearchCase::IgnoreCase))
		{
			return EUIClickType::Right;
		}
		if (InString.Equals(TEXT("Middle"), ESearchCase::IgnoreCase))
		{
			return EUIClickType::Middle;
		}
		return EUIClickType::Left;
	}
};
