// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/GSCharacterBase.h"
#include "GSHeroCharacter.generated.h"

/**
 * A player or AI controlled hero character.
 */
UCLASS()
class GASSHOOTER_API AGSHeroCharacter : public AGSCharacterBase
{
	GENERATED_BODY()
	
public:
	AGSHeroCharacter(const class FObjectInitializer& ObjectInitializer);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|Camera")
	float BaseTurnRate = 45.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|Camera")
	float BaseLookUpRate = 45.0f;

	bool ASCInputBound = false;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Mouse
	void LookUp(float Value);

	// Gamepad
	void LookUpRate(float Value);

	// Mouse
	void Turn(float Value);

	// Gamepad
	void TurnRate(float Value);

	// Mouse + Gamepad
	void MoveForward(float Value);

	// Mouse + Gamepad
	void MoveRight(float Value);

	// Creates and initializes the floating status bar for heroes.
	// Safe to call many times because it checks to make sure it only executes once.
	UFUNCTION()
	void InitializeFloatingStatusBar();

	// Client only
	virtual void OnRep_PlayerState() override;

	// Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the PlayerController might
	// call ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	// Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart so the Actor's InputComponent would be null in OnRep_PlayerState.
	void BindASCInput();
};
