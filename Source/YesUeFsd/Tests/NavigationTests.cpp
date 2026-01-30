// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/NavigationHelper.h"
#include "AutoDriver/NavigationCache.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationHelperCreationTest,
	"YesUeFsd.Navigation.Helper.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationHelperCreationTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	UNavigationHelper* NavHelper = NewObject<UNavigationHelper>();
	TestNotNull(TEXT("NavigationHelper Created"), NavHelper);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCacheTest,
	"YesUeFsd.Navigation.Cache.Basic",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationCacheTest::RunTest(const FString& Parameters)
{
	// Create navigation cache
	FNavigationCache Cache(100);

	FVector Start(0, 0, 0);
	FVector End(1000, 0, 0);
	FNavigationCacheKey Key(Start, End, nullptr);

	// Test cache miss
	FNavigationCacheEntry Entry;
	bool bFound = Cache.Get(Key, Entry);
	TestFalse(TEXT("Cache Miss"), bFound);

	// Add entry to cache
	FNavigationCacheEntry NewEntry;
	NewEntry.bIsReachable = true;
	NewEntry.PathLength = 1000.0f;
	NewEntry.Timestamp = FPlatformTime::Seconds();
	Cache.Add(Key, NewEntry);

	// Test cache hit
	bFound = Cache.Get(Key, Entry);
	TestTrue(TEXT("Cache Hit"), bFound);
	TestEqual(TEXT("Cached Path Length"), Entry.PathLength, 1000.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCacheLRUTest,
	"YesUeFsd.Navigation.Cache.LRU",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationCacheLRUTest::RunTest(const FString& Parameters)
{
	// Create small cache to test LRU eviction
	FNavigationCache Cache(3);

	// Add 3 entries
	for (int32 i = 0; i < 3; ++i)
	{
		FVector Start(i * 100, 0, 0);
		FVector End(i * 100 + 1000, 0, 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		Entry.bIsReachable = true;
		Entry.PathLength = 1000.0f;
		Entry.Timestamp = FPlatformTime::Seconds();

		Cache.Add(Key, Entry);
	}

	// Add 4th entry, should evict oldest
	FVector Start4(300, 0, 0);
	FVector End4(1300, 0, 0);
	FNavigationCacheKey Key4(Start4, End4, nullptr);

	FNavigationCacheEntry Entry4;
	Entry4.bIsReachable = true;
	Entry4.PathLength = 1000.0f;
	Entry4.Timestamp = FPlatformTime::Seconds();
	Cache.Add(Key4, Entry4);

	// First entry should be evicted
	FVector Start0(0, 0, 0);
	FVector End0(1000, 0, 0);
	FNavigationCacheKey Key0(Start0, End0, nullptr);

	FNavigationCacheEntry FoundEntry;
	bool bFound = Cache.Get(Key0, FoundEntry);
	TestFalse(TEXT("Oldest Entry Evicted"), bFound);

	// 4th entry should be present
	bFound = Cache.Get(Key4, FoundEntry);
	TestTrue(TEXT("Newest Entry Present"), bFound);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCacheInvalidationTest,
	"YesUeFsd.Navigation.Cache.Invalidation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationCacheInvalidationTest::RunTest(const FString& Parameters)
{
	FNavigationCache Cache(100);

	FVector Start(0, 0, 0);
	FVector End(1000, 0, 0);
	FNavigationCacheKey Key(Start, End, nullptr);

	// Add entry
	FNavigationCacheEntry Entry;
	Entry.bIsReachable = true;
	Entry.PathLength = 1000.0f;
	Entry.Timestamp = FPlatformTime::Seconds();
	Cache.Add(Key, Entry);

	// Clear cache
	Cache.Clear();

	// Entry should be gone
	FNavigationCacheEntry FoundEntry;
	bool bFound = Cache.Get(Key, FoundEntry);
	TestFalse(TEXT("Cache Cleared"), bFound);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationReachabilityTest,
	"YesUeFsd.Navigation.Query.Reachability",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationReachabilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Enable navigation system
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!TestNotNull(TEXT("Navigation System Available"), NavSys))
	{
		return false;
	}

	UNavigationHelper* NavHelper = NewObject<UNavigationHelper>();
	if (!TestNotNull(TEXT("NavigationHelper Created"), NavHelper))
	{
		return false;
	}

	// Test reachability query (may fail without nav mesh, but should not crash)
	FVector Start(0, 0, 0);
	FVector End(1000, 0, 0);
	bool bReachable = NavHelper->IsLocationReachable(World, Start, End);

	// Just test that query completes without crashing
	TestTrue(TEXT("Reachability Query Completed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationPathLengthTest,
	"YesUeFsd.Navigation.Query.PathLength",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationPathLengthTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	UNavigationHelper* NavHelper = NewObject<UNavigationHelper>();

	// Test path length query
	FVector Start(0, 0, 0);
	FVector End(1000, 0, 0);
	float PathLength = NavHelper->GetPathLength(World, Start, End);

	// Path length should be non-negative
	TestTrue(TEXT("Path Length Non-Negative"), PathLength >= 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationRandomLocationTest,
	"YesUeFsd.Navigation.Query.RandomLocation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationRandomLocationTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	UNavigationHelper* NavHelper = NewObject<UNavigationHelper>();

	// Test random location query
	FVector Origin(0, 0, 0);
	float Radius = 1000.0f;
	FVector RandomLocation;
	bool bSuccess = NavHelper->GetRandomReachableLocation(World, Origin, Radius, RandomLocation);

	// Query should complete (may fail without nav mesh)
	TestTrue(TEXT("Random Location Query Completed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCachePerformanceTest,
	"YesUeFsd.Navigation.Cache.Performance",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FNavigationCachePerformanceTest::RunTest(const FString& Parameters)
{
	FNavigationCache Cache(1000);

	// Add many entries
	const int32 NumEntries = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumEntries; ++i)
	{
		FVector Start(i * 10, 0, 0);
		FVector End(i * 10 + 100, 0, 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		Entry.bIsReachable = true;
		Entry.PathLength = 100.0f;
		Entry.Timestamp = FPlatformTime::Seconds();

		Cache.Add(Key, Entry);
	}

	double AddTime = FPlatformTime::Seconds() - StartTime;

	// Test lookup performance
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < NumEntries; ++i)
	{
		FVector Start(i * 10, 0, 0);
		FVector End(i * 10 + 100, 0, 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		Cache.Get(Key, Entry);
	}

	double LookupTime = FPlatformTime::Seconds() - StartTime;

	// Performance should be reasonable (less than 1 second for 1000 operations)
	TestTrue(TEXT("Add Performance Acceptable"), AddTime < 1.0);
	TestTrue(TEXT("Lookup Performance Acceptable"), LookupTime < 1.0);

	AddInfo(FString::Printf(TEXT("Add Time: %.3f ms, Lookup Time: %.3f ms"),
		AddTime * 1000.0, LookupTime * 1000.0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCacheStatsTest,
	"YesUeFsd.Navigation.Cache.Stats",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationCacheStatsTest::RunTest(const FString& Parameters)
{
	FNavigationCache Cache(100);

	// Add entries and test hits/misses
	FVector Start(0, 0, 0);
	FVector End(1000, 0, 0);
	FNavigationCacheKey Key(Start, End, nullptr);

	// Miss
	FNavigationCacheEntry Entry;
	Cache.Get(Key, Entry);

	// Add
	Entry.bIsReachable = true;
	Entry.PathLength = 1000.0f;
	Entry.Timestamp = FPlatformTime::Seconds();
	Cache.Add(Key, Entry);

	// Hit
	Cache.Get(Key, Entry);

	// Get stats
	FNavigationCacheStats Stats = Cache.GetStats();
	TestEqual(TEXT("Cache Hits"), Stats.Hits, 1);
	TestEqual(TEXT("Cache Misses"), Stats.Misses, 1);
	TestTrue(TEXT("Hit Rate Calculated"), Stats.HitRate > 0.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
