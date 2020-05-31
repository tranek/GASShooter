// Copyright 2020 Dan Kestranek.


#include "Characters/GSASCActorBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"

// Sets default values
AGSASCActorBase::AGSASCActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Minimal mode means GameplayEffects are not replicated to anyone. Only GameplayTags and Attributes are replicated to clients.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

UAbilitySystemComponent* AGSASCActorBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AGSASCActorBase::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
