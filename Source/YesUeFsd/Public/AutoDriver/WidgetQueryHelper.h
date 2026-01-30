// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/AutoDriverUITypes.h"

class UWidget;
class UUserWidget;
class UWorld;

/**
 * Widget Query Helper
 *
 * Static utility class for querying UMG widget trees.
 * Provides various methods to find and inspect widgets in the UI hierarchy.
 *
 * Usage:
 *   FWidgetInfo Info = UWidgetQueryHelper::FindWidgetByName(World, "StartButton");
 *   if (Info.IsValid()) { ... }
 */
class YESUEFSD_API UWidgetQueryHelper
{
public:
	// ========================================
	// Widget Finding
	// ========================================

	/**
	 * Find a single widget matching the query parameters
	 * @param World World context
	 * @param QueryParams Query parameters
	 * @return Widget info (bFound will be false if not found)
	 */
	static FWidgetInfo FindWidget(UWorld* World, const FWidgetQueryParams& QueryParams);

	/**
	 * Find all widgets matching the query parameters
	 * @param World World context
	 * @param QueryParams Query parameters
	 * @return Array of widget info
	 */
	static TArray<FWidgetInfo> FindWidgets(UWorld* World, const FWidgetQueryParams& QueryParams);

	/**
	 * Find a widget by exact name
	 * @param World World context
	 * @param WidgetName Widget name to search for
	 * @return Widget info (bFound will be false if not found)
	 */
	static FWidgetInfo FindWidgetByName(UWorld* World, const FString& WidgetName);

	/**
	 * Find widgets containing specific text
	 * @param World World context
	 * @param SearchText Text to search for
	 * @param bExactMatch If true, text must match exactly
	 * @return Array of widgets containing the text
	 */
	static TArray<FWidgetInfo> FindWidgetsByText(UWorld* World, const FString& SearchText, bool bExactMatch = false);

	/**
	 * Find all visible buttons in the UI
	 * @param World World context
	 * @return Array of button widget info
	 */
	static TArray<FWidgetInfo> FindAllButtons(UWorld* World);

	// ========================================
	// Widget Information
	// ========================================

	/**
	 * Get detailed information about a widget
	 * @param Widget Widget to inspect
	 * @return Widget info structure
	 */
	static FWidgetInfo GetWidgetInfo(UWidget* Widget);

	/**
	 * Extract text content from a widget
	 * @param Widget Widget to extract text from
	 * @return Text content (empty if widget has no text)
	 */
	static FString GetWidgetText(UWidget* Widget);

	/**
	 * Get widget screen position and size
	 * @param Widget Widget to query
	 * @param OutPosition Screen position (top-left)
	 * @param OutSize Widget size
	 * @return True if geometry was retrieved successfully
	 */
	static bool GetWidgetGeometry(UWidget* Widget, FVector2D& OutPosition, FVector2D& OutSize);

	/**
	 * Check if a widget is visible (including all parent visibility)
	 * @param Widget Widget to check
	 * @return True if widget and all parents are visible
	 */
	static bool IsWidgetVisible(UWidget* Widget);

	/**
	 * Check if a widget is enabled/interactive
	 * @param Widget Widget to check
	 * @return True if widget is enabled
	 */
	static bool IsWidgetEnabled(UWidget* Widget);

	/**
	 * Get the full widget path in the hierarchy
	 * @param Widget Widget to get path for
	 * @return Full widget path (e.g., "MainMenu/ButtonPanel/StartButton")
	 */
	static FString GetWidgetPath(UWidget* Widget);

	// ========================================
	// Widget Tree Traversal
	// ========================================

	/**
	 * Get all active UUserWidget instances in the world
	 * @param World World context
	 * @return Array of active user widgets
	 */
	static TArray<UUserWidget*> GetAllActiveUserWidgets(UWorld* World);

	/**
	 * Get all child widgets recursively
	 * @param ParentWidget Parent widget to start from
	 * @param bRecursive If true, get all descendants; if false, only direct children
	 * @return Array of child widgets
	 */
	static TArray<UWidget*> GetAllChildWidgets(UWidget* ParentWidget, bool bRecursive = true);

	/**
	 * Find widget by predicate
	 * @param World World context
	 * @param Predicate Function that returns true for matching widget
	 * @return First widget matching the predicate
	 */
	static UWidget* FindWidgetByPredicate(UWorld* World, TFunction<bool(UWidget*)> Predicate);

	/**
	 * Find all widgets by predicate
	 * @param World World context
	 * @param Predicate Function that returns true for matching widgets
	 * @return Array of widgets matching the predicate
	 */
	static TArray<UWidget*> FindAllWidgetsByPredicate(UWorld* World, TFunction<bool(UWidget*)> Predicate);

	// ========================================
	// Widget Matching
	// ========================================

	/**
	 * Check if a widget matches the given query parameters
	 * @param Widget Widget to check
	 * @param QueryParams Query parameters to match against
	 * @return True if widget matches the query
	 */
	static bool MatchesQuery(UWidget* Widget, const FWidgetQueryParams& QueryParams);

private:

	/**
	 * Helper to recursively traverse widget tree
	 */
	static void TraverseWidgetTree(UWidget* Widget, TFunction<void(UWidget*)> Callback, bool bRecursive = true);

	/**
	 * Helper to get widgets from a UserWidget's WidgetTree
	 */
	static void GetWidgetsFromUserWidget(UUserWidget* UserWidget, TArray<UWidget*>& OutWidgets, bool bRecursive = true);
};
