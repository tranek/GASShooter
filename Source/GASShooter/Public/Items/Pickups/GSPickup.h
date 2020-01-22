// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSPickup.generated.h"

UCLASS()
class GASSHOOTER_API AGSPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AGSPickup();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class UCapsuleComponent* CollisionComp;
};
