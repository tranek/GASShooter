// Copyright 2020 Dan Kestranek.


#include "Items/Pickups/GSPickup.h"
#include "Components/CapsuleComponent.h"
#include "GASShooter/GASShooter.h"

// Sets default values
AGSPickup::AGSPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComp"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;
}
