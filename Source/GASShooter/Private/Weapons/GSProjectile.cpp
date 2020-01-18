// Copyright 2020 Dan Kestranek.


#include "Weapons/GSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGSProjectile::AGSProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->InitialSpeed = 7000.0f;

	bReplicates = true;

	//TODO change this to a better value
	NetUpdateFrequency = 100.0f;
}
