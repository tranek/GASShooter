// Copyright 2024 Dan Kestranek.


#include "GSEngineSubsystem.h"
#include "AbilitySystemGlobals.h"

void UGSEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAbilitySystemGlobals::Get().InitGlobalData();
}
