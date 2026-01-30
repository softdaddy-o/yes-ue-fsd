// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/AutoDriverUITypes.h"

class UWidget;
class UWorld;

/**
 * UI Interaction Helper
 *
 * Static utility class for simulating UI interactions.
 * Provides methods to click, hover, type text, and scroll widgets.
 *
 * Usage:
 *   bool Success = UUIInteractionHelper::ClickWidgetByName(World, "StartButton");
 */
class YESUEFSD_API UUIInteractionHelper
{
public:
	// ========================================
	// Click Operations
	// ========================================

	/**
	 * Click a widget
	 * @param World World context
	 * @param Widget Widget to click
	 * @param ClickParams Click parameters
	 * @return True if click was simulated successfully
	 */
	static bool ClickWidget(UWorld* World, UWidget* Widget, const FUIClickParams& ClickParams = FUIClickParams());

	/**
	 * Click a widget by name
	 * @param World World context
	 * @param WidgetName Name of widget to click
	 * @param ClickParams Click parameters
	 * @return True if widget was found and clicked
	 */
	static bool ClickWidgetByName(UWorld* World, const FString& WidgetName, const FUIClickParams& ClickParams = FUIClickParams());

	/**
	 * Click at a specific screen position
	 * @param World World context
	 * @param ScreenPosition Screen position to click
	 * @param ClickParams Click parameters
	 * @return True if click was simulated successfully
	 */
	static bool ClickAtScreenPosition(UWorld* World, const FVector2D& ScreenPosition, const FUIClickParams& ClickParams = FUIClickParams());

	// ========================================
	// Hover Operations
	// ========================================

	/**
	 * Hover over a widget
	 * @param World World context
	 * @param Widget Widget to hover
	 * @return True if hover was simulated successfully
	 */
	static bool HoverWidget(UWorld* World, UWidget* Widget);

	/**
	 * Hover over a widget by name
	 * @param World World context
	 * @param WidgetName Name of widget to hover
	 * @return True if widget was found and hovered
	 */
	static bool HoverWidgetByName(UWorld* World, const FString& WidgetName);

	/**
	 * Move mouse to a specific screen position
	 * @param World World context
	 * @param ScreenPosition Target screen position
	 * @return True if mouse was moved successfully
	 */
	static bool MoveMouseToPosition(UWorld* World, const FVector2D& ScreenPosition);

	// ========================================
	// Text Input Operations
	// ========================================

	/**
	 * Set text in a widget (EditableText, EditableTextBox, etc.)
	 * @param World World context
	 * @param Widget Widget to set text in
	 * @param Text Text to set
	 * @return True if text was set successfully
	 */
	static bool SetTextInWidget(UWorld* World, UWidget* Widget, const FString& Text);

	/**
	 * Set text in a widget by name
	 * @param World World context
	 * @param WidgetName Name of widget to set text in
	 * @param Text Text to set
	 * @return True if widget was found and text was set
	 */
	static bool SetTextInWidgetByName(UWorld* World, const FString& WidgetName, const FString& Text);

	/**
	 * Focus a widget (give it keyboard focus)
	 * @param World World context
	 * @param Widget Widget to focus
	 * @return True if widget was focused successfully
	 */
	static bool FocusWidget(UWorld* World, UWidget* Widget);

	// ========================================
	// Scroll Operations
	// ========================================

	/**
	 * Scroll a widget into view
	 * @param World World context
	 * @param Widget Widget to scroll into view
	 * @return True if widget was scrolled into view
	 */
	static bool ScrollWidgetIntoView(UWorld* World, UWidget* Widget);

	/**
	 * Scroll a scroll box by delta
	 * @param World World context
	 * @param Widget ScrollBox widget
	 * @param ScrollDelta Amount to scroll (positive = down/right)
	 * @return True if scroll was successful
	 */
	static bool ScrollWidget(UWorld* World, UWidget* Widget, float ScrollDelta);

private:
	/**
	 * Helper to simulate a single mouse button event
	 */
	static bool SimulateMouseButtonEvent(UWorld* World, const FVector2D& ScreenPosition, EUIClickType ClickType, bool bIsPressed);

	/**
	 * Helper to get the game viewport client
	 */
	static class UGameViewportClient* GetGameViewportClient(UWorld* World);

	/**
	 * Helper to convert EUIClickType to Slate mouse button
	 */
	static struct FKey GetMouseButtonKey(EUIClickType ClickType);

	/**
	 * Helper to get widget center position
	 */
	static bool GetWidgetCenterPosition(UWidget* Widget, FVector2D& OutPosition);
};
