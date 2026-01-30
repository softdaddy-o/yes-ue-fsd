// Copyright Yes UE FSD. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WaypointComponent.generated.h"

/**
 * Waypoint component for defining navigation points and patrol routes.
 *
 * Usage:
 * - Attach to actors in your level to mark navigation waypoints
 * - Chain waypoints together using NextWaypoint
 * - Use with AutoDriver for automated patrol behavior
 */
UCLASS(ClassGroup=(AutoDriver), meta=(BlueprintSpawnableComponent), Blueprintable)
class YESUEFSD_API UWaypointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWaypointComponent();

	// Waypoint identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FString WaypointName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	int32 WaypointIndex;

	// Visual properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Visual")
	FLinearColor WaypointColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Visual")
	bool bShowDebugSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Visual")
	float DebugSphereRadius;

	// Behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Behavior")
	bool bAutoProceedToNext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Behavior")
	float WaitTimeAtWaypoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Behavior")
	UWaypointComponent* NextWaypoint;

	// Optional rotation target at waypoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Behavior")
	bool bHasRotationTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Behavior", meta = (EditCondition = "bHasRotationTarget"))
	FRotator TargetRotation;

	// Actions to perform at waypoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Actions")
	TArray<FName> ButtonsToPress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Actions")
	FString CustomActionTag;

	/**
	 * Get the world location of this waypoint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	FVector GetWaypointLocation() const;

	/**
	 * Get the next waypoint in the sequence.
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	UWaypointComponent* GetNextWaypoint() const;

	/**
	 * Check if this waypoint has a next waypoint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	bool HasNextWaypoint() const;

	/**
	 * Get the distance to another waypoint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	float GetDistanceToWaypoint(UWaypointComponent* OtherWaypoint) const;

	/**
	 * Draw debug visualization for this waypoint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Waypoint|Debug")
	void DrawDebugWaypoint(float Duration = 5.0f) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

/**
 * Actor that holds a waypoint component for easier level placement.
 */
UCLASS(Blueprintable, ClassGroup=(AutoDriver))
class YESUEFSD_API AWaypointActor : public AActor
{
	GENERATED_BODY()

public:
	AWaypointActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waypoint")
	UWaypointComponent* WaypointComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waypoint")
	UStaticMeshComponent* MeshComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waypoint")
	class UBillboardComponent* SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waypoint")
	class UArrowComponent* ArrowComponent;
#endif
};

/**
 * Helper class for managing waypoint routes.
 */
UCLASS(Blueprintable)
class YESUEFSD_API UWaypointRoute : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString RouteName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<UWaypointComponent*> Waypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	bool bLoopRoute;

	/**
	 * Get waypoint at index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Route")
	UWaypointComponent* GetWaypointAtIndex(int32 Index) const;

	/**
	 * Get total number of waypoints.
	 */
	UFUNCTION(BlueprintCallable, Category = "Route")
	int32 GetWaypointCount() const;

	/**
	 * Get total route length.
	 */
	UFUNCTION(BlueprintCallable, Category = "Route")
	float GetTotalRouteLength() const;

	/**
	 * Find the nearest waypoint to a location.
	 */
	UFUNCTION(BlueprintCallable, Category = "Route")
	UWaypointComponent* FindNearestWaypoint(const FVector& Location) const;

	/**
	 * Validate the route (check all waypoints are valid).
	 */
	UFUNCTION(BlueprintCallable, Category = "Route")
	bool IsRouteValid() const;
};
