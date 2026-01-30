// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/NavigationHelper.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

bool UNavigationHelper::IsLocationReachable(
	UObject* WorldContextObject,
	const FVector& From,
	const FVector& To,
	const FVector& QueryExtent)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return false;
	}

	// Test path existence
	FPathFindingQuery Query;
	Query.StartLocation = From;
	Query.EndLocation = To;
	Query.NavData = NavSys->GetDefaultNavDataInstance();

	if (!Query.NavData.IsValid())
	{
		return false;
	}

	FPathFindingResult Result = NavSys->FindPathSync(Query);
	return Result.IsSuccessful() && Result.Path.IsValid();
}

bool UNavigationHelper::IsLocationOnNavMesh(
	UObject* WorldContextObject,
	const FVector& Location,
	const FVector& QueryExtent)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return false;
	}

	FNavLocation NavLocation;
	return NavSys->ProjectPointToNavigation(Location, NavLocation, QueryExtent);
}

FNavigationQueryResult UNavigationHelper::ProjectLocationToNavMesh(
	UObject* WorldContextObject,
	const FVector& Location,
	const FVector& QueryExtent)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return FNavigationQueryResult::Failure(TEXT("Navigation system not available"));
	}

	FNavLocation NavLocation;
	if (NavSys->ProjectPointToNavigation(Location, NavLocation, QueryExtent))
	{
		return FNavigationQueryResult::Success(NavLocation.Location);
	}

	return FNavigationQueryResult::Failure(TEXT("Could not project location to navmesh"));
}

FNavigationQueryResult UNavigationHelper::GetPathLength(
	UObject* WorldContextObject,
	const FVector& From,
	const FVector& To)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return FNavigationQueryResult::Failure(TEXT("Navigation system not available"));
	}

	FPathFindingQuery Query;
	Query.StartLocation = From;
	Query.EndLocation = To;
	Query.NavData = NavSys->GetDefaultNavDataInstance();

	if (!Query.NavData.IsValid())
	{
		return FNavigationQueryResult::Failure(TEXT("No navigation data"));
	}

	FPathFindingResult Result = NavSys->FindPathSync(Query);
	if (Result.IsSuccessful() && Result.Path.IsValid())
	{
		float PathLength = Result.Path->GetLength();
		return FNavigationQueryResult::Success(To, PathLength);
	}

	return FNavigationQueryResult::Failure(TEXT("Path not found"));
}

float UNavigationHelper::GetStraightLineDistance(const FVector& From, const FVector& To)
{
	return FVector::Dist(From, To);
}

FNavigationQueryResult UNavigationHelper::GetRandomReachableLocation(
	UObject* WorldContextObject,
	const FVector& Origin,
	float Radius)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return FNavigationQueryResult::Failure(TEXT("Navigation system not available"));
	}

	FNavLocation NavLocation;
	if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLocation))
	{
		return FNavigationQueryResult::Success(NavLocation.Location);
	}

	return FNavigationQueryResult::Failure(TEXT("No reachable location found"));
}

FNavigationQueryResult UNavigationHelper::GetRandomLocationInRadius(
	UObject* WorldContextObject,
	const FVector& Origin,
	float Radius)
{
	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return FNavigationQueryResult::Failure(TEXT("Navigation system not available"));
	}

	FNavLocation NavLocation;
	if (NavSys->GetRandomPointInNavigableRadius(Origin, Radius, NavLocation))
	{
		return FNavigationQueryResult::Success(NavLocation.Location);
	}

	return FNavigationQueryResult::Failure(TEXT("No location found"));
}

void UNavigationHelper::DrawDebugPath(
	UObject* WorldContextObject,
	const FVector& From,
	const FVector& To,
	float Duration,
	FLinearColor Color)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		// Draw straight line if no nav system
		DrawDebugLine(World, From, To, Color.ToFColor(true), false, Duration, 0, 3.0f);
		return;
	}

	FPathFindingQuery Query;
	Query.StartLocation = From;
	Query.EndLocation = To;
	Query.NavData = NavSys->GetDefaultNavDataInstance();

	if (!Query.NavData.IsValid())
	{
		return;
	}

	FPathFindingResult Result = NavSys->FindPathSync(Query);
	if (Result.IsSuccessful() && Result.Path.IsValid())
	{
		const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();

		// Draw path segments
		for (int32 i = 0; i < PathPoints.Num() - 1; i++)
		{
			DrawDebugLine(
				World,
				PathPoints[i].Location,
				PathPoints[i + 1].Location,
				Color.ToFColor(true),
				false,
				Duration,
				0,
				3.0f
			);
		}

		// Draw start and end points
		DrawDebugSphere(World, From, 25.0f, 12, FColor::Green, false, Duration);
		DrawDebugSphere(World, To, 25.0f, 12, FColor::Red, false, Duration);
	}
	else
	{
		// Draw failed path in red
		DrawDebugLine(World, From, To, FColor::Red, false, Duration, 0, 3.0f);
		DrawDebugSphere(World, From, 25.0f, 12, FColor::Yellow, false, Duration);
		DrawDebugSphere(World, To, 25.0f, 12, FColor::Orange, false, Duration);
	}
}

void UNavigationHelper::DrawDebugNavMesh(
	UObject* WorldContextObject,
	const FVector& Location,
	float Radius,
	float Duration)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	UNavigationSystemV1* NavSys = GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		return;
	}

	// Draw a sphere to show the query area
	DrawDebugSphere(World, Location, Radius, 32, FColor::Cyan, false, Duration, 0, 2.0f);

	// Note: Drawing actual navmesh polygons requires more complex geometry queries
	// For now, we just visualize the query area
}

UNavigationSystemV1* UNavigationHelper::GetNavigationSystem(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	return FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
}

bool UNavigationHelper::IsNavigationSystemAvailable(UObject* WorldContextObject)
{
	return GetNavigationSystem(WorldContextObject) != nullptr;
}
