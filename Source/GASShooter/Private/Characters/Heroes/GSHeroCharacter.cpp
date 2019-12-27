// Copyright 2019 Dan Kestranek.


#include "Characters/Heroes/GSHeroCharacter.h"
#include "AI/GSHeroAIController.h"
#include "Camera/CameraComponent.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GASShooter/GASShooterGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/GSPlayerController.h"
#include "Player/GSPlayerState.h"
#include "UI/GSFloatingStatusBarWidget.h"

AGSHeroCharacter::AGSHeroCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ThirdPersonCameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	ThirdPersonCameraBoom->SetupAttachment(RootComponent);
	ThirdPersonCameraBoom->bUsePawnControlRotation = true;
	ThirdPersonCameraBoom->SetRelativeLocation(FVector(0, 60, 68.492264));

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonCameraBoom);
	ThirdPersonCamera->FieldOfView = 80.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetVisibility(false, true);

	// Makes sure that the animations play on the Server so that we can use bone and socket transforms
	// to do things like spawning projectiles and other FX.
	//GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	GetMesh()->bCastHiddenShadow = true;

	ThirdPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("ThirdPersonWeaponMesh"));
	ThirdPersonWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThirdPersonWeaponMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonWeaponMesh->bCastHiddenShadow = true;
	
	FirstPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonWeaponMesh"));
	FirstPersonWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonWeaponMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonWeaponMesh->CastShadow = false;
	FirstPersonWeaponMesh->SetVisibility(false, true);

	UIFloatingStatusBarComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIFloatingStatusBarComponent"));
	UIFloatingStatusBarComponent->SetupAttachment(RootComponent);
	UIFloatingStatusBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	UIFloatingStatusBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	UIFloatingStatusBarComponent->SetDrawSize(FVector2D(500, 500));

	UIFloatingStatusBarClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/GASShooter/UI/UI_FloatingStatusBar_Hero.UI_FloatingStatusBar_Hero_C"));
	if (!UIFloatingStatusBarClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find UIFloatingStatusBarClass. If it was moved, please update the reference location in C++."), TEXT(__FUNCTION__));
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AGSHeroAIController::StaticClass();
}

// Called to bind functionality to input
void AGSHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGSHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGSHeroCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AGSHeroCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGSHeroCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AGSHeroCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGSHeroCharacter::TurnRate);

	PlayerInputComponent->BindAction("TogglePerspective", IE_Pressed, this, &AGSHeroCharacter::TogglePerspective);

	// Bind player input to the AbilitySystemComponent. Also called in OnRep_PlayerState because of a potential race condition.
	BindASCInput();
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

		// If player is host on listen server, the floating status bar would have been created for them from BeginPlay before player possession, hide it
		if (IsLocallyControlled() && IsPlayerControlled() && UIFloatingStatusBarComponent && UIFloatingStatusBar)
		{
			UIFloatingStatusBarComponent->SetVisibility(false, true);
		}


		// Respawn specific things that won't affect first possession.

		// Forcibly set the DeadTag count to 0
		AbilitySystemComponent->SetTagMapCount(DeadTag, 0);

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
	}
}

void AGSHeroCharacter::Restart()
{
	Super::Restart();

	SetPerspective(IsFirstPersonPerspective);
}

UGSFloatingStatusBarWidget* AGSHeroCharacter::GetFloatingStatusBar()
{
	return UIFloatingStatusBar;
}

void AGSHeroCharacter::Die()
{
	// Go into third person for death animation
	SetPerspective(false);

	Super::Die();
}

void AGSHeroCharacter::FinishDying()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AGASShooterGameModeBase* GM = Cast<AGASShooterGameModeBase>(GetWorld()->GetAuthGameMode());

		if (GM)
		{
			GM->HeroDied(GetController());
		}
	}

	Super::FinishDying();
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
	//StartingCameraBoomArmLength = CameraBoom->TargetArmLength;
	//StartingCameraBoomLocation = CameraBoom->GetRelativeLocation();
}

void AGSHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetMesh() && ThirdPersonWeaponMesh)
	{
		ThirdPersonWeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("WeaponPoint"));
		ThirdPersonWeaponMesh->SetRelativeRotation(FRotator(0, 0, -90.0f));
	}

	if (FirstPersonMesh && FirstPersonWeaponMesh)
	{
		FirstPersonWeaponMesh->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("WeaponPoint"));
		FirstPersonWeaponMesh->SetRelativeRotation(FRotator(0, 0, -90.0f));
	}
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
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AGSHeroCharacter::MoveRight(float Value)
{
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AGSHeroCharacter::TogglePerspective()
{
	IsFirstPersonPerspective = !IsFirstPersonPerspective;
	SetPerspective(IsFirstPersonPerspective);
}

void AGSHeroCharacter::SetPerspective(bool InIsFirstPersonPerspective)
{
	// Only change perspective for the locally controlled player. Simulated proxies should stay in third person.
	// To swap cameras, deactivate current camera (defaults to ThirdPersonCamera), activate desired camera, and call PlayerController->SetViewTarget() on self
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	if (PC && PC->IsLocalPlayerController())
	{
		if (InIsFirstPersonPerspective)
		{
			ThirdPersonCamera->Deactivate();
			FirstPersonCamera->Activate();
			PC->SetViewTarget(this);

			GetMesh()->SetVisibility(false, true);
			FirstPersonMesh->SetVisibility(true, true);

			// Move third person mesh back so that the shadow doesn't look disconnected
			GetMesh()->AddLocalOffset(FVector(0.0f, -100.0f, 0.0f));
		}
		else
		{
			FirstPersonCamera->Deactivate();
			ThirdPersonCamera->Activate();
			PC->SetViewTarget(this);

			FirstPersonMesh->SetVisibility(false, true);
			GetMesh()->SetVisibility(true, true);

			// Reset the third person mesh
			GetMesh()->AddLocalOffset(FVector(0.0f, 100.0f, 0.0f));
		}
	}
}

void AGSHeroCharacter::InitializeFloatingStatusBar()
{
	// Only create once
	if (UIFloatingStatusBar || !IsValid(AbilitySystemComponent))
	{
		return;
	}

	// Don't create for locally controlled player. We could add a game setting to toggle this later.
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		return;
	}

	// Need a valid PlayerState
	if (!GetPlayerState())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	AGSPlayerController* PC = Cast<AGSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UGSFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				UIFloatingStatusBar->SetManaPercentage(GetMana() / GetMaxMana());
				UIFloatingStatusBar->OwningCharacter = this;
			}
		}
	}
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

		// Init ASC Actor Info for clients. Server will init its AI/PlayerController when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
		BindASCInput();

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

		// Forcibly set the DeadTag count to 0
		AbilitySystemComponent->SetTagMapCount(DeadTag, 0);

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
	}
}

void AGSHeroCharacter::BindASCInput()
{
	if (!ASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), FString("EGSAbilityInputID"), static_cast<int32>(EGSAbilityInputID::Confirm), static_cast<int32>(EGSAbilityInputID::Cancel)));

		ASCInputBound = true;
	}
}
