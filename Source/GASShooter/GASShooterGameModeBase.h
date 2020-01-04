// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GASShooterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API AGASShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGASShooterGameModeBase();

	void HeroDied(AController* Controller);

protected:
	float RespawnDelay;

	TSubclassOf<class AGSHeroCharacter> HeroClass;

	AActor* EnemySpawnPoint;

	virtual void BeginPlay() override;

	void RespawnHero(AController* Controller);
};
