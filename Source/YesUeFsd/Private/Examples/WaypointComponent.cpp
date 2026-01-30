// Copyright Yes UE FSD. All Rights Reserved.

#include "Examples/WaypointComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UWaypointComponent::UWaypointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	WaypointName = TEXT("Waypoint");
	WaypointIndex = 0;
	WaypointColor = FLinearColor::Green;
	bShowDebugSphere = true;
	DebugSphereRadius = 50.0f;
	bAutoProceedToNext = true;
	WaitTimeAtWaypoint = 1.0f;
	NextWaypoint = nullptr;
	bHasRotationTarget = false;
	TargetRotation = FRotator::ZeroRotator;
}

void UWaypointComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bShowDebugSphere)
	{
		DrawDebugWaypoint(999999.0f); // Draw for very long time
	}
}

void UWaypointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#if WITH_EDITOR
void UWaypointComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Refresh debug visualization when properties change
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UWaypointComponent, WaypointColor) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UWaypointComponent, bShowDebugSphere) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UWaypointComponent, DebugSphereRadius))
		{
			// Property changed, visualization will update on next draw
		}
	}
}
#endif

FVector UWaypointComponent::GetWaypointLocation() const
{
	return GetComponentLocation();
}

UWaypointComponent* UWaypointComponent::GetNextWaypoint() const
{
	return NextWaypoint;
}

bool UWaypointComponent::HasNextWaypoint() const
{
	return NextWaypoint != nullptr;
}

float UWaypointComponent::GetDistanceToWaypoint(UWaypointComponent* OtherWaypoint) const
{
	if (!OtherWaypoint)
	{
		return -1.0f;
	}

	return FVector::Dist(GetWaypointLocation(), OtherWaypoint->GetWaypointLocation());
}

void UWaypointComponent::DrawDebugWaypoint(float Duration) const
{
	if (!GetWorld())
	{
		return;
	}

	FVector Location = GetWaypointLocation();

	// Draw sphere
	if (bShowDebugSphere)
	{
		DrawDebugSphere(GetWorld(), Location, DebugSphereRadius, 16, WaypointColor.ToFColor(true), false, Duration, 0, 2.0f);
	}

	// Draw index number
	DrawDebugString(GetWorld(), Location + FVector(0, 0, DebugSphereRadius + 20.0f),
		FString::Printf(TEXT("%s [%d]"), *WaypointName, WaypointIndex),
		nullptr, WaypointColor.ToFColor(true), Duration, true);

	// Draw arrow to next waypoint
	if (NextWaypoint)
	{
		FVector NextLocation = NextWaypoint->GetWaypointLocation();
		DrawDebugDirectionalArrow(GetWorld(), Location, NextLocation, 20.0f, WaypointColor.ToFColor(true), false, Duration, 0, 2.0f);
	}

	// Draw rotation target if specified
	if (bHasRotationTarget)
	{
		FVector Forward = TargetRotation.Vector() * 100.0f;
		DrawDebugDirectionalArrow(GetWorld(), Location, Location + Forward, 15.0f, FColor::Yellow, false, Duration, 0, 1.5f);
	}
}

// AWaypointActor implementation

AWaypointActor::AWaypointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create waypoint component
	WaypointComponent = CreateDefaultSubobject<UWaypointComponent>(TEXT("WaypointComponent"));
	WaypointComponent->SetupAttachment(RootComponent);

	// Create visual mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Try to load a default sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMeshFinder.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.5f)); // Make it smaller
	}

#if WITH_EDITORONLY_DATA
	// Billboard for editor visibility
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SpriteComponent"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->bIsScreenSizeScaled = true;
	}

	// Arrow showing forward direction
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	if (ArrowComponent)
	{
		ArrowComponent->SetupAttachment(RootComponent);
		ArrowComponent->ArrowColor = FColor::Green;
		ArrowComponent->bIsScreenSizeScaled = true;
	}
#endif
}

// UWaypointRoute implementation

UWaypointComponent* UWaypointRoute::GetWaypointAtIndex(int32 Index) const
{
	if (Index >= 0 && Index < Waypoints.Num())
	{
		return Waypoints[Index];
	}
	return nullptr;
}

int32 UWaypointRoute::GetWaypointCount() const
{
	return Waypoints.Num();
}

float UWaypointRoute::GetTotalRouteLength() const
{
	float TotalLength = 0.0f;

	for (int32 i = 0; i < Waypoints.Num() - 1; ++i)
	{
		if (Waypoints[i] && Waypoints[i + 1])
		{
			TotalLength += Waypoints[i]->GetDistanceToWaypoint(Waypoints[i + 1]);
		}
	}

	// Add loop distance if route loops
	if (bLoopRoute && Waypoints.Num() > 1 && Waypoints[0] && Waypoints.Last())
	{
		TotalLength += Waypoints.Last()->GetDistanceToWaypoint(Waypoints[0]);
	}

	return TotalLength;
}

UWaypointComponent* UWaypointRoute::FindNearestWaypoint(const FVector& Location) const
{
	UWaypointComponent* NearestWaypoint = nullptr;
	float NearestDistance = MAX_FLT;

	for (UWaypointComponent* Waypoint : Waypoints)
	{
		if (!Waypoint)
		{
			continue;
		}

		float Distance = FVector::Dist(Location, Waypoint->GetWaypointLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestWaypoint = Waypoint;
		}
	}

	return NearestWaypoint;
}

bool UWaypointRoute::IsRouteValid() const
{
	if (Waypoints.Num() == 0)
	{
		return false;
	}

	// Check all waypoints are valid
	for (UWaypointComponent* Waypoint : Waypoints)
	{
		if (!Waypoint)
		{
			return false;
		}
	}

	return true;
}
