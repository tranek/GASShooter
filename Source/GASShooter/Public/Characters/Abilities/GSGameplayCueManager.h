// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "GSGameplayCueManager.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()
	
public:
	// This makes it so GameplayCues will load the first time that they're requested (or use your AssetManager to manually load them).
	// By default it loads *every* GameplayCue in the project and all of their referenced assets when the map starts. In a large game
	// with lots of GCs, this could be hundreds of megabytes or more of unused assets loaded in RAM if particular GCs are not used in
	// the current map.
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override
	{
		//return true; // Default	
		return false;
	}
};
