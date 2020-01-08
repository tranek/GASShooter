// Copyright 2020 Dan Kestranek.


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
#include "Net/UnrealNetwork.h"
#include "Player/GSPlayerController.h"
#include "Player/GSPlayerState.h"
#include "TimerManager.h"
#include "UI/GSFloatingStatusBarWidget.h"
#include "Weapons/GSWeapon.h"

AGSHeroCharacter::AGSHeroCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bIsFirstPersonPerspective = false;
	bASCInputBound = false;
	Default1PFOV = 90.0f;
	Default3PFOV = 80.0f;
	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	CurrentWeaponTag = NoWeaponTag;
	
	ThirdPersonCameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	ThirdPersonCameraBoom->SetupAttachment(RootComponent);
	ThirdPersonCameraBoom->bUsePawnControlRotation = true;
	ThirdPersonCameraBoom->SetRelativeLocation(FVector(0, 50, 68.492264));

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonCameraBoom);
	ThirdPersonCamera->FieldOfView = Default3PFOV;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetVisibility(false, true);

	// Makes sure that the animations play on the Server so that we can use bone and socket transforms
	// to do things like spawning projectiles and other FX.
	//GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	GetMesh()->bCastHiddenShadow = true;

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

	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	AIControllerClass = AGSHeroAIController::StaticClass();
}

void AGSHeroCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSHeroCharacter, Inventory);
	DOREPLIFETIME(AGSHeroCharacter, CurrentWeapon);
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

	SetupStartupPerspective();
}

void AGSHeroCharacter::Restart()
{
	Super::Restart();

	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *GetName(), ACTOR_ROLE_FSTRING);

	if (IsLocallyControlled() && IsPlayerControlled())
	{
		bIsFirstPersonPerspective = true;
	}

	SetPerspective(bIsFirstPersonPerspective);
}

UGSFloatingStatusBarWidget* AGSHeroCharacter::GetFloatingStatusBar()
{
	return UIFloatingStatusBar;
}

void AGSHeroCharacter::Die()
{
	// Go into third person for death animation
	SetPerspective(false);

	RemoveAllWeaponsFromInventory();

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

bool AGSHeroCharacter::IsInFirstPersonPerspective() const
{
	return bIsFirstPersonPerspective;
}

USkeletalMeshComponent* AGSHeroCharacter::GetFirstPersonMesh()
{
	return FirstPersonMesh;
}

USkeletalMeshComponent* AGSHeroCharacter::GetThirdPersonMesh()
{
	return GetMesh();
}

bool AGSHeroCharacter::AddWeaponToInventory(AGSWeapon* NewWeapon)
{
	if (DoesWeaponExistInInventory(NewWeapon))
	{
		//TODO grab ammo from it if we're not full and destroy it in the world

		return false;
	}

	Inventory.Weapons.Add(NewWeapon);
	NewWeapon->SetOwningCharacter(this);
	NewWeapon->AddAbilities();

	return true;
}

bool AGSHeroCharacter::RemoveWeaponFromInventory(AGSWeapon* WeaponToRemove)
{
	if (DoesWeaponExistInInventory(WeaponToRemove))
	{
		WeaponToRemove->RemoveAbilities();
		WeaponToRemove->SetOwningCharacter(nullptr);
		WeaponToRemove->ResetWeapon();
		Inventory.Weapons.Remove(WeaponToRemove);

		//TODO check if equipped and unequip it if it is
		// probably need a function IsEquipped() or something either here or on the weapon or both?
	}

	return false;
}

void AGSHeroCharacter::RemoveAllWeaponsFromInventory()
{
	UnEquipCurrentWeapon();

	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	for (int32 i = Inventory.Weapons.Num() - 1; i >= 0; i--)
	{
		AGSWeapon* Weapon = Inventory.Weapons[i];
		RemoveWeaponFromInventory(Weapon);
	}
}

void AGSHeroCharacter::EquipWeapon(AGSWeapon* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s Equipping %s"), TEXT(__FUNCTION__), *NewWeapon->GetName());

		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void AGSHeroCharacter::ServerEquipWeapon_Implementation(AGSWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool AGSHeroCharacter::ServerEquipWeapon_Validate(AGSWeapon* NewWeapon)
{
	return true;
}

FName AGSHeroCharacter::GetWeaponAttachPoint()
{
	return WeaponAttachPoint;
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

	Inventory = FGSHeroInventory();
}

void AGSHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StartingThirdPersonCameraBoomArmLength = ThirdPersonCameraBoom->TargetArmLength;
	StartingThirdPersonCameraBoomLocation = ThirdPersonCameraBoom->GetRelativeLocation();
	StartingThirdPersonMeshLocation = GetMesh()->GetRelativeLocation();

	GetWorldTimerManager().SetTimerForNextTick(this, &AGSHeroCharacter::SpawnDefaultInventory);
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
	bIsFirstPersonPerspective = !bIsFirstPersonPerspective;
	SetPerspective(bIsFirstPersonPerspective);
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
			GetMesh()->SetRelativeLocation(StartingThirdPersonMeshLocation);
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
				UIFloatingStatusBar->SetCharacterName(CharacterName);
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

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
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

		// If current weapon repped before PlayerState, set tag on ASC
		if (CurrentWeapon)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
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

void AGSHeroCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	SetupStartupPerspective();
}

void AGSHeroCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), FString("EGSAbilityInputID"), static_cast<int32>(EGSAbilityInputID::Confirm), static_cast<int32>(EGSAbilityInputID::Cancel)));

		bASCInputBound = true;
	}
}

void AGSHeroCharacter::SpawnDefaultInventory()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s"), TEXT(__FUNCTION__), *GetName());

	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryWeaponClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryWeaponClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AGSWeapon* NewWeapon = GetWorld()->SpawnActor<AGSWeapon>(DefaultInventoryWeaponClasses[i], SpawnInfo);
			AddWeaponToInventory(NewWeapon);
		}
	}

	// Equip first weapon in inventory
	if (Inventory.Weapons.Num() > 0)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
}

void AGSHeroCharacter::SetupStartupPerspective()
{
	if (IsLocallyControlled() && IsPlayerControlled())
	{
		bIsFirstPersonPerspective = true;
	}

	SetPerspective(bIsFirstPersonPerspective);
}

bool AGSHeroCharacter::DoesWeaponExistInInventory(AGSWeapon* InWeapon)
{
	UE_LOG(LogTemp, Log, TEXT("%s InWeapon class %s"), TEXT(__FUNCTION__), *InWeapon->GetClass()->GetName());

	for (AGSWeapon* Weapon : Inventory.Weapons)
	{
		if (Weapon->GetClass() == InWeapon->GetClass())
		{
			return true;
		}
	}

	return false;
}

void AGSHeroCharacter::SetCurrentWeapon(AGSWeapon* NewWeapon, AGSWeapon* LastWeapon)
{
	UnEquipCurrentWeapon();

	if (NewWeapon)
	{
		if (AbilitySystemComponent)
		{
			// Clear out potential NoWeaponTag
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}

		NewWeapon->Equip();
		CurrentWeapon = NewWeapon;
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		AGSPlayerController* PC = GetController<AGSPlayerController>();
		if (PC && PC->IsLocalController())
		{
			PC->SetEquippedWeaponPrimaryIconFromSprite(CurrentWeapon->PrimaryIcon);
			PC->SetEquippedWeaponStatusText(CurrentWeapon->StatusText);
			PC->SetEquippedWeaponClipAmmo(CurrentWeapon->GetClipAmmo());
			PC->SetEquippedWeaponReserveAmmo(CurrentWeapon->GetReserveAmmo());
		}
	}
}

void AGSHeroCharacter::UnEquipCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->UnEquip();
		CurrentWeapon = nullptr;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	AGSPlayerController* PC = GetController<AGSPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetEquippedWeaponPrimaryIconFromSprite(nullptr);
		PC->SetEquippedWeaponStatusText(FText());
		PC->SetEquippedWeaponClipAmmo(0);
		PC->SetEquippedWeaponReserveAmmo(0);
	}
}

void AGSHeroCharacter::OnRep_CurrentWeapon(AGSWeapon* LastWeapon)
{
	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *GetName(), ACTOR_ROLE_FSTRING);

	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}
