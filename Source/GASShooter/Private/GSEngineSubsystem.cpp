// Copyright 2020 Dan Kestranek.


#include "GSEngineSubsystem.h"
#include "AbilitySystemGlobals.h"

void UGSEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAbilitySystemGlobals::Get().InitGlobalData();
}
