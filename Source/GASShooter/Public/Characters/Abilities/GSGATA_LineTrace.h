// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldCollision.h"
#include "GSGATA_LineTrace.generated.h"

/**
 * Reusable, configurable line trace TargetActor.
 * Meant to be used with GSAT_WaitTargetDataUsingActor instead of the default WaitTargetData AbilityTask as the default
 * one will destroy the TargetActor.
 */
UCLASS()
class GASSHOOTER_API AGSGATA_LineTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGSGATA_LineTrace();

	// Base weapon spread (degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float WeaponSpread;

	// Aiming spread modifier
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float AimingSpreadMod;

	// Continuous firing: spread increment
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float FiringSpreadIncrement;

	// Continuous firing: max increment
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float FiringSpreadMax;

	// Current spread from continuous firing
	float CurrentFiringSpread;

	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	bool bUseAimingSpreadMod;

	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	FGameplayTag AimingTag;
	
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	FGameplayTag AimingRemovalTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	float MaxRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, meta = (ExposeOnSpawn = true), Category = "Trace")
	FCollisionProfileName TraceProfile;

	// Does the trace affect the aiming pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bTraceAffectsAimPitch;

	// Maximum hit results to return. 0 just returns the trace end point. < 0 returns infinite hit results.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	int32 MaxHitResults;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bIgnoreBlockingHits;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bTraceFromPlayerViewPoint;

	UFUNCTION(BlueprintCallable)
	void ResetSpread();

	float GetCurrentSpread() const;

	/**
	* Configure the TargetActor for use. This TargetActor could be used in multiple abilities and there's no guarantee
	* what state it will be in. You will need to make sure that only one ability is using this TargetActor at a time.
	*
	* @param InStartLocation Location to trace from.
	* @param InAimingTag Optional. Predicted GameplayTag for aiming. Only used if we mofify spread while aiming. If used,
	* must set InAimingRemovalTag also.
	* @param InAimingRemovalTag Optional. Predicted GameplayTag for aiming removal. Only used if we mofify spread while
	* aiming. If used, must set InAimingTag also.
	* @param InTraceProfile Collision profile to use for tracing.
	* @param InFilter Hit Actors must pass this filter to be returned in the TargetData.
	* @param InReticleClass Reticle that will appear on top of acquired targets. Reticles will be spawned/despawned as targets are acquired/lost.
	* @param InReticleParams Parameters for world reticle. Usage of these parameters is dependent on the reticle.
	* @param bInIgnoreBlockingHits Ignore blocking collision hits in the trace. Useful if you want to target through walls.
	* @param bInShouldProduceTargetDataOnServer If set, this TargetActor will produce TargetData on the Server in addition
	* to the client and the client will just send a generic "Confirm" event to the server. If false, the client will send
	* the TargetData to the Server. This is handled by the WaitTargetDataUsingActor AbilityTask.
	* @param bInDebug When true, this TargetActor will show debug lines of the trace and hit results.
	* @param bInTraceAffectsAimPitch Does the trace affect the aiming pitch?
	* @param bInTraceFromPlayerViewPoint Should we trace from the player ViewPoint instead of the StartLocation? The
	* TargetData HitResults will still have the StartLocation for the TraceStart. This is useful for FPS where we want
	* to trace from the player ViewPoint but draw the bullet tracer from the weapon muzzle.
	* TODO: AI Controllers should fall back to muzzle location. Not implemented yet.
	* @param bInUseAImingSpreadMod Should we modify spread based on if we're aiming? If true, must set InAimingTag and
	* InAimingRemovalTag.
	* @param InMaxRange Max range for this trace.
	* @param InWeaponSpread Base weapon spread in degrees.
	* @param InAimingSpreadMod Optional. Multiplicative modifier to spread if aiming.
	* @param InFiringSpreadIncrement Amount spread increments from continuous fire in degrees.
	* @param InFiringSpreadMax Maximum amount of spread for continuous fire in degrees.
	* @param InMaxHitResults Max hit results that a trace can return. 0 just returns the trace end point. < 0 returns infinite
	* hit results.
	*/
	UFUNCTION(BlueprintCallable)
	void Configure(
		UPARAM(DisplayName = "Start Location") const FGameplayAbilityTargetingLocationInfo& InStartLocation,
		UPARAM(DisplayName = "Aiming Tag") FGameplayTag InAimingTag,
		UPARAM(DisplayName = "Aiming Removal Tag") FGameplayTag InAimingRemovalTag,
		UPARAM(DisplayName = "Trace Profile") FCollisionProfileName InTraceProfile,
		UPARAM(DisplayName = "Filter") FGameplayTargetDataFilterHandle InFilter,
		UPARAM(DisplayName = "Reticle Class") TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
		UPARAM(DisplayName = "Reticle Params") FWorldReticleParameters InReticleParams,
		UPARAM(DisplayName = "Ignore Blocking Hits") bool bInIgnoreBlockingHits = false,
		UPARAM(DisplayName = "Should Produce Target Data on Server") bool bInShouldProduceTargetDataOnServer = false,
		UPARAM(DisplayName = "Debug") bool bInDebug = false,
		UPARAM(DisplayName = "Trace Affects Aim Pitch") bool bInTraceAffectsAimPitch = true,
		UPARAM(DisplayName = "Trace From Player ViewPoint") bool bInTraceFromPlayerViewPoint = false,
		UPARAM(DisplayName = "Use Aiming Spread Mod") bool bInUseAimingSpreadMod = false,
		UPARAM(DisplayName = "Max Range") float InMaxRange = 999999.0f,
		UPARAM(DisplayName = "Weapon Spread") float InWeaponSpread = 0.0f,
		UPARAM(DisplayName = "Aiming Spread Mod") float InAimingSpreadMod = 0.0f,
		UPARAM(DisplayName = "Firing Spread Increment") float InFiringSpreadIncrement = 0.0f,
		UPARAM(DisplayName = "Firing Spread Max") float InFiringSpreadMax = 0.0f,
		UPARAM(DisplayName = "Max Hit Results") int32 InMaxHitResults = 1
	);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetDestroyOnConfirmation(bool bInDestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Traces as normal, but will manually filter all hit actors
	void LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch = false);

	bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

protected:
	TArray<TWeakObjectPtr<AGameplayAbilityWorldReticle>> ReticleActors;

	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitResults) const;
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);

#if ENABLE_DRAW_DEBUG
	// Util for drawing result of multi line trace from KismetTraceUtils.h
	void DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType, bool bHit, const TArray<FHitResult>& OutHits, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);
#endif // ENABLE_DRAW_DEBUG
};
