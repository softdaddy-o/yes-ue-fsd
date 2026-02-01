// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

/**
 * Navigation Query Cache
 *
 * LRU cache for navigation query results to avoid redundant pathfinding calculations.
 * Thread-safe for use from multiple threads.
 */
class YESUEFSD_API FNavigationQueryCache
{
public:
	/** Cache entry storing path finding results */
	struct FCacheEntry
	{
		FVector StartLocation;
		FVector EndLocation;
		FNavigationPath* Path;
		float PathLength;
		bool bIsValid;
		double Timestamp;

		FCacheEntry()
			: StartLocation(FVector::ZeroVector)
			, EndLocation(FVector::ZeroVector)
			, Path(nullptr)
			, PathLength(0.0f)
			, bIsValid(false)
			, Timestamp(0.0)
		{}

		FCacheEntry(const FVector& InStart, const FVector& InEnd, FNavigationPath* InPath, float InLength, double InTimestamp)
			: StartLocation(InStart)
			, EndLocation(InEnd)
			, Path(InPath)
			, PathLength(InLength)
			, bIsValid(InPath != nullptr && InPath->IsValid())
			, Timestamp(InTimestamp)
		{}

		bool IsStillValid() const
		{
			return bIsValid && Path != nullptr && Path->IsValid();
		}
	};

	FNavigationQueryCache(int32 InMaxCacheSize = 128, float InCacheTolerance = 100.0f)
		: MaxCacheSize(InMaxCacheSize)
		, CacheTolerance(InCacheTolerance)
	{}

	/**
	 * Find a cached path result
	 * @param From Starting location
	 * @param To Ending location
	 * @param OutEntry Output cache entry if found
	 * @return True if valid cached entry found
	 */
	bool FindCachedPath(const FVector& From, const FVector& To, FCacheEntry& OutEntry);

	/**
	 * Add a path result to the cache
	 * @param From Starting location
	 * @param To Ending location
	 * @param Path Path result (can be null for failed paths) - UE 5.7: Changed to FNavigationPath*
	 * @param PathLength Length of the path
	 */
	void CachePath(const FVector& From, const FVector& To, FNavigationPath* Path, float PathLength);

	/**
	 * Clear the cache
	 */
	void Clear();

	/**
	 * Get cache statistics
	 */
	void GetCacheStats(int32& OutHits, int32& OutMisses, int32& OutEntries) const
	{
		FScopeLock Lock(&CacheMutex);
		OutHits = CacheHits;
		OutMisses = CacheMisses;
		OutEntries = CacheEntries.Num();
	}

	/**
	 * Reset statistics
	 */
	void ResetStats()
	{
		FScopeLock Lock(&CacheMutex);
		CacheHits = 0;
		CacheMisses = 0;
	}

private:
	/** Generate cache key from two locations */
	uint64 GenerateCacheKey(const FVector& From, const FVector& To) const;

	/** Check if two locations are close enough to be considered the same */
	bool LocationsMatch(const FVector& A, const FVector& B) const
	{
		return FVector::DistSquared(A, B) <= (CacheTolerance * CacheTolerance);
	}

	/** Evict oldest entries when cache is full */
	void EvictOldestEntry();

	/** Cache storage (key -> entry) */
	TMap<uint64, FCacheEntry> CacheEntries;

	/** Maximum number of cache entries */
	int32 MaxCacheSize;

	/** Distance tolerance for cache matching (cm) */
	float CacheTolerance;

	/** Cache statistics */
	mutable int32 CacheHits = 0;
	mutable int32 CacheMisses = 0;

	/** Thread safety */
	mutable FCriticalSection CacheMutex;
};
