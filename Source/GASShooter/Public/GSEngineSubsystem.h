// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "GSEngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
