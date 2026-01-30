// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NavigationHelper.generated.h"

class UNavigationSystemV1;
class ANavigationData;

/**
 * Navigation query result
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FNavigationQueryResult
{
	GENERATED_BODY()

	/** Was the query successful */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	bool bSuccess = false;

	/** Resulting location (for queries that return a location) */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FVector Location = FVector::ZeroVector;

	/** Path length in units (for path queries) */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float PathLength = 0.0f;

	/** Error message if query failed */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FString ErrorMessage;

	FNavigationQueryResult() = default;

	FNavigationQueryResult(bool bInSuccess, const FString& InError = TEXT(""))
		: bSuccess(bInSuccess), ErrorMessage(InError)
	{
	}

	static FNavigationQueryResult Success(const FVector& InLocation = FVector::ZeroVector, float InPathLength = 0.0f)
	{
		FNavigationQueryResult Result;
		Result.bSuccess = true;
		Result.Location = InLocation;
		Result.PathLength = InPathLength;
		return Result;
	}

	static FNavigationQueryResult Failure(const FString& InError)
	{
		return FNavigationQueryResult(false, InError);
	}
};

/**
 * Navigation Helper
 *
 * Utility class for navigation system queries and operations.
 * Provides convenient access to Unreal's navigation system for
 * pathfinding, reachability checks, and navigation mesh queries.
 */
UCLASS(BlueprintType)
class YESUEFSD_API UNavigationHelper : public UObject
{
	GENERATED_BODY()

public:
	// ========================================
	// Reachability Queries
	// ========================================

	/**
	 * Check if a location is reachable from another location
	 * @param World World context
	 * @param From Starting location
	 * @param To Target location
	 * @param QueryExtent Size of the query box
	 * @return True if location is reachable
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static bool IsLocationReachable(
		UObject* WorldContextObject,
		const FVector& From,
		const FVector& To,
		const FVector& QueryExtent = FVector(50, 50, 50));

	/**
	 * Check if a location is on the navigation mesh
	 * @param WorldContextObject World context
	 * @param Location Location to check
	 * @param QueryExtent Size of the query box
	 * @return True if location is on navmesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static bool IsLocationOnNavMesh(
		UObject* WorldContextObject,
		const FVector& Location,
		const FVector& QueryExtent = FVector(50, 50, 50));

	/**
	 * Project a location onto the navigation mesh
	 * @param WorldContextObject World context
	 * @param Location Location to project
	 * @param QueryExtent Size of the query box
	 * @return Query result with projected location
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static FNavigationQueryResult ProjectLocationToNavMesh(
		UObject* WorldContextObject,
		const FVector& Location,
		const FVector& QueryExtent = FVector(500, 500, 500));

	// ========================================
	// Path Queries
	// ========================================

	/**
	 * Get the path length between two locations
	 * @param WorldContextObject World context
	 * @param From Starting location
	 * @param To Target location
	 * @return Query result with path length
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static FNavigationQueryResult GetPathLength(
		UObject* WorldContextObject,
		const FVector& From,
		const FVector& To);

	/**
	 * Get the straight-line distance between two locations
	 * @param From Starting location
	 * @param To Target location
	 * @return Distance in units
	 */
	UFUNCTION(BlueprintPure, Category = "Navigation Helper")
	static float GetStraightLineDistance(const FVector& From, const FVector& To);

	// ========================================
	// Random Location Queries
	// ========================================

	/**
	 * Find a random reachable location within a radius
	 * @param WorldContextObject World context
	 * @param Origin Center point
	 * @param Radius Search radius
	 * @return Query result with random location
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static FNavigationQueryResult GetRandomReachableLocation(
		UObject* WorldContextObject,
		const FVector& Origin,
		float Radius = 1000.0f);

	/**
	 * Find a random location on the navigation mesh within a radius
	 * @param WorldContextObject World context
	 * @param Origin Center point
	 * @param Radius Search radius
	 * @return Query result with random location
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static FNavigationQueryResult GetRandomLocationInRadius(
		UObject* WorldContextObject,
		const FVector& Origin,
		float Radius = 1000.0f);

	// ========================================
	// Debug Visualization
	// ========================================

	/**
	 * Draw debug path between two locations
	 * @param WorldContextObject World context
	 * @param From Starting location
	 * @param To Target location
	 * @param Duration How long to display (0 = one frame)
	 * @param Color Path color
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static void DrawDebugPath(
		UObject* WorldContextObject,
		const FVector& From,
		const FVector& To,
		float Duration = 2.0f,
		FLinearColor Color = FLinearColor::Green);

	/**
	 * Draw debug navigation mesh around a location
	 * @param WorldContextObject World context
	 * @param Location Center location
	 * @param Radius Visualization radius
	 * @param Duration How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static void DrawDebugNavMesh(
		UObject* WorldContextObject,
		const FVector& Location,
		float Radius = 500.0f,
		float Duration = 2.0f);

	// ========================================
	// Utility
	// ========================================

	/**
	 * Get the navigation system for a world
	 * @param WorldContextObject World context
	 * @return Navigation system, or nullptr if not available
	 */
	UFUNCTION(BlueprintPure, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static UNavigationSystemV1* GetNavigationSystem(UObject* WorldContextObject);

	/**
	 * Check if navigation system is available
	 * @param WorldContextObject World context
	 * @return True if navigation is available
	 */
	UFUNCTION(BlueprintPure, Category = "Navigation Helper", meta = (WorldContext = "WorldContextObject"))
	static bool IsNavigationSystemAvailable(UObject* WorldContextObject);
};
