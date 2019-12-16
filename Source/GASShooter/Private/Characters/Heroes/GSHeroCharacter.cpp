// Copyright 2019 Dan Kestranek.


#include "GSHeroCharacter.h"
#include "GSAbilitySystemComponent.h"
#include "GSAttributeSetBase.h"
#include "GSPlayerController.h"
#include "GSPlayerState.h"
#include "Kismet/KismetMathLibrary.h"

AGSHeroCharacter::AGSHeroCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//TODO
}

// Called to bind functionality to input
void AGSHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGSHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveBackward", this, &AGSHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGSHeroCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveLeft", this, &AGSHeroCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AGSHeroCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGSHeroCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AGSHeroCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGSHeroCharacter::TurnRate);

	// Bind to AbilitySystemComponent
	AbilitySystemComponent->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
		FString("CancelTarget"), FString("EGDAbilityInputID"), static_cast<int32>(EGSAbilityInputID::Confirm), static_cast<int32>(EGSAbilityInputID::Cancel)));
}

// Server only
void AGSHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AGSPlayerState* PS = GetPlayerState<AGSPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

		AddStartupEffects();

		AddCharacterAbilities();

		AGSPlayerController* PC = Cast<AGSPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		InitializeFloatingStatusBar();


		// Respawn specific things that won't affect first possession.

		//TODO
		// Forcibly set the DeadTag count to 0
		//AbilitySystemComponent->SetTagMapCount(DeadTag, 0);

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
	}
}

/**
* On the Server, Possession happens before BeginPlay.
* On the Client, BeginPlay happens before Possession.
* So we can't use BeginPlay to do anything with the AbilitySystemComponent because we don't have it until the PlayerState replicates from possession.
*/
void AGSHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Only needed for Heroes placed in world and when the player is the Server.
	// On respawn, they are set up in PossessedBy.
	// When the player a client, the floating status bars are all set up in OnRep_PlayerState.
	InitializeFloatingStatusBar();

	//TODO
	//GunComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("GunSocket"));

	//StartingCameraBoomArmLength = CameraBoom->TargetArmLength;
	//StartingCameraBoomLocation = CameraBoom->GetRelativeLocation();
}

void AGSHeroCharacter::LookUp(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value);
	}
}

void AGSHeroCharacter::LookUpRate(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AGSHeroCharacter::Turn(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value);
	}
}

void AGSHeroCharacter::TurnRate(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value * BaseTurnRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AGSHeroCharacter::MoveForward(float Value)
{
	AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
}

void AGSHeroCharacter::MoveRight(float Value)
{
	AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
}

void AGSHeroCharacter::InitializeFloatingStatusBar()
{
	//TODO

	/*
	// Only create once
	if (UIFloatingStatusBar || !AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	AGDPlayerController* PC = Cast<AGDPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UGDFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				UIFloatingStatusBar->SetManaPercentage(GetMana() / GetMaxMana());
			}
		}
	}
	*/
}

// Client only
void AGSHeroCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AGSPlayerState* PS = GetPlayerState<AGSPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Refresh ASC Actor Info for clients. Server will be refreshed by its AI/PlayerController when it possesses a new Actor.
		AbilitySystemComponent->RefreshAbilityActorInfo();

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

		AGSPlayerController* PC = Cast<AGSPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
		InitializeFloatingStatusBar();


		// Respawn specific things that won't affect first possession.

		//TODO
		// Forcibly set the DeadTag count to 0
		//AbilitySystemComponent->SetTagMapCount(DeadTag, 0);

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
	}
}