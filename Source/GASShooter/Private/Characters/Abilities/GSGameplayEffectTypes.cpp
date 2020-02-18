// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGameplayEffectTypes.h"

bool FGSGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bool bSuccess = Super::NetSerialize(Ar, Map, bOutSuccess);

	bSuccess = bSuccess && TargetData.NetSerialize(Ar, Map, bOutSuccess);

	return bSuccess;
}
