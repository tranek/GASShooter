// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSProjectile.generated.h"

UCLASS()
class GASSHOOTER_API AGSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSProjectile();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PBProjectile")
	class UProjectileMovementComponent* ProjectileMovement;
};
