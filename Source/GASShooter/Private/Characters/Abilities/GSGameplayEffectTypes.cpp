// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGameplayEffectTypes.h"

bool FGSGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
}
