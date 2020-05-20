// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSInteractable.h"

bool IGSInteractable::IsAvailableForInteraction_Implementation() const
{
	return false;
}

float IGSInteractable::GetInteractDuration_Implementation() const
{
	return 0.0f;
}
