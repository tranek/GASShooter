// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSHUDReticle.generated.h"

/**
 * A reticle on the player's HUD. Can be changed by weapons, abilities, etc.
 */
UCLASS()
class GASSHOOTER_API UGSHUDReticle : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Essentially an interface that all child classes in Blueprint will have to fill out
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColor(FLinearColor Color);
};
