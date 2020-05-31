// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GSASCActorBase.generated.h"

/**
* Base AActor class that has an AbilitySystemComponent but does not inherit from ACharacter.
*/
UCLASS()
class GASSHOOTER_API AGSASCActorBase : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSASCActorBase();

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	class UGSAbilitySystemComponent* AbilitySystemComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
