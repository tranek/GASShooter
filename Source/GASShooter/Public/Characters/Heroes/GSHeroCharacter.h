// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/GSCharacterBase.h"
#include "GSHeroCharacter.generated.h"

class AGSWeapon;

USTRUCT()
struct GASSHOOTER_API FGSHeroInventory
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<AGSWeapon*> Weapons;

	// Consumable items

	// Passive items like armor

	// Door keys

	// Etc
};

/**
 * A player or AI controlled hero character.
 */
UCLASS()
class GASSHOOTER_API AGSHeroCharacter : public AGSCharacterBase
{
	GENERATED_BODY()
	
public:
	AGSHeroCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

	virtual void Restart() override;

	class UGSFloatingStatusBarWidget* GetFloatingStatusBar();

	virtual void Die() override;

	virtual void FinishDying() override;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSHeroCharacter")
	virtual bool IsInFirstPersonPerspective() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSHeroCharacter")
	USkeletalMeshComponent* GetFirstPersonMesh();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSHeroCharacter")
	USkeletalMeshComponent* GetThirdPersonMesh();

	// Adds a new weapon to the inventory.
	// Returns false if the weapon already exists in the inventory, true if it's a new weapon.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Inventory")
	bool AddWeaponToInventory(AGSWeapon* NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Inventory")
	void EquipWeapon(AGSWeapon* InWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(AGSWeapon* InWeapon);
	void ServerEquipWeapon_Implementation(AGSWeapon* InWeapon);
	bool ServerEquipWeapon_Validate(AGSWeapon* InWeapon);

	FName GetWeaponAttachPoint();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|Camera")
	float BaseTurnRate = 45.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|Camera")
	float BaseLookUpRate = 45.0f;

	// Default to first person
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|Camera")
	bool bIsFirstPersonPerspective = false;

	bool bASCInputBound = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSHeroCharacter")
	FName WeaponAttachPoint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASShooter|Camera")
	class USpringArmComponent* ThirdPersonCameraBoom;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASShooter|Camera")
	class UCameraComponent* ThirdPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASShooter|Camera")
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class UGSFloatingStatusBarWidget> UIFloatingStatusBarClass;

	UPROPERTY()
	class UGSFloatingStatusBarWidget* UIFloatingStatusBar;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASShooter|UI")
	class UWidgetComponent* UIFloatingStatusBarComponent;

	UPROPERTY(Replicated)
	FGSHeroInventory Inventory;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|Inventory")
	TArray<TSubclassOf<AGSWeapon>> DefaultInventoryWeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AGSWeapon* CurrentWeapon;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

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

	// Toggles between perspectives
	void TogglePerspective();

	// Sets the perspective
	void SetPerspective(bool Is1PPerspective);

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

	// Server spawns default inventory
	void SpawnDefaultInventory();

	bool DoesWeaponExistInInventory(AGSWeapon* InWeapon);

	// Unequips the current weapon
	void UnEquipCurrentWeapon();

	UFUNCTION()
	void OnRep_CurrentWeapon();
};
