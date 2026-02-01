// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/NavigationCache.h"
#include "HAL/PlatformTime.h"

bool FNavigationQueryCache::FindCachedPath(const FVector& From, const FVector& To, FCacheEntry& OutEntry)
{
	FScopeLock Lock(&CacheMutex);

	uint64 Key = GenerateCacheKey(From, To);

	if (FCacheEntry* Entry = CacheEntries.Find(Key))
	{
		// Verify locations still match within tolerance
		if (LocationsMatch(Entry->StartLocation, From) && LocationsMatch(Entry->EndLocation, To))
		{
			// Check if cached path is still valid
			if (Entry->IsStillValid())
			{
				OutEntry = *Entry;
				// Update timestamp for LRU
				Entry->Timestamp = FPlatformTime::Seconds();
				CacheHits++;
				return true;
			}
			else
			{
				// Invalid entry, remove it
				CacheEntries.Remove(Key);
			}
		}
	}

	CacheMisses++;
	return false;
}

void FNavigationQueryCache::CachePath(const FVector& From, const FVector& To, FNavigationPath* Path, float PathLength)
{
	FScopeLock Lock(&CacheMutex);

	// Evict old entries if cache is full
	if (CacheEntries.Num() >= MaxCacheSize)
	{
		EvictOldestEntry();
	}

	uint64 Key = GenerateCacheKey(From, To);
	double Timestamp = FPlatformTime::Seconds();

	CacheEntries.Add(Key, FCacheEntry(From, To, Path, PathLength, Timestamp));
}

void FNavigationQueryCache::Clear()
{
	FScopeLock Lock(&CacheMutex);
	CacheEntries.Empty();
	CacheHits = 0;
	CacheMisses = 0;
}

uint64 FNavigationQueryCache::GenerateCacheKey(const FVector& From, const FVector& To) const
{
	// Round to tolerance to improve cache hits for nearby queries
	FVector RoundedFrom = (From / CacheTolerance).GridSnap(1.0f) * CacheTolerance;
	FVector RoundedTo = (To / CacheTolerance).GridSnap(1.0f) * CacheTolerance;

	// Simple hash combination
	uint64 Hash = 0;
	Hash ^= GetTypeHash(RoundedFrom.X);
	Hash ^= (uint64(GetTypeHash(RoundedFrom.Y)) << 16);
	Hash ^= (uint64(GetTypeHash(RoundedFrom.Z)) << 32);
	Hash ^= (uint64(GetTypeHash(RoundedTo.X)) << 8);
	Hash ^= (uint64(GetTypeHash(RoundedTo.Y)) << 24);
	Hash ^= (uint64(GetTypeHash(RoundedTo.Z)) << 40);

	return Hash;
}

void FNavigationQueryCache::EvictOldestEntry()
{
	if (CacheEntries.Num() == 0)
	{
		return;
	}

	// Find entry with oldest timestamp
	uint64 OldestKey = 0;
	double OldestTime = TNumericLimits<double>::Max();

	for (const auto& Pair : CacheEntries)
	{
		if (Pair.Value.Timestamp < OldestTime)
		{
			OldestTime = Pair.Value.Timestamp;
			OldestKey = Pair.Key;
		}
	}

	CacheEntries.Remove(OldestKey);
}
