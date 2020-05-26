// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSInteractable.h"

bool IGSInteractable::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	return false;
}

float IGSInteractable::GetInteractDuration_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	return 0.0f;
}

void IGSInteractable::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

void IGSInteractable::GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

FSimpleMulticastDelegate* IGSInteractable::GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent)
{
	return nullptr;
}
