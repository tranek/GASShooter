// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Characters/GSCharacterBase.h"
#include "GSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API AGSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void CreateHUD();

	class UGSHUDWidget* GetGSHUD();

	UFUNCTION(Client, Reliable, WithValidation)
	void ShowDamageNumber(float DamageAmount, AGSCharacterBase* TargetCharacter);
	void ShowDamageNumber_Implementation(float DamageAmount, AGSCharacterBase* TargetCharacter);
	bool ShowDamageNumber_Validate(float DamageAmount, AGSCharacterBase* TargetCharacter);

	// Simple way to RPC to the client the countdown until they respawn from the GameMode. Will be latency amount of out sync with the Server.
	UFUNCTION(Client, Reliable, WithValidation)
	void SetRespawnCountdown(float RespawnTimeRemaining);
	void SetRespawnCountdown_Implementation(float RespawnTimeRemaining);
	bool SetRespawnCountdown_Validate(float RespawnTimeRemaining);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class UGSHUDWidget> UIHUDWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "GASShooter|UI")
	class UGSHUDWidget* UIHUDWidget;

	// Server only
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_PlayerState() override;
};
