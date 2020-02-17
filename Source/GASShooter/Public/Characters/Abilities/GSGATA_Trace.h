// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldCollision.h"
#include "GSGATA_Trace.generated.h"

/**
 * Reusable, configurable trace TargetActor. Subclass with your own trace shapes.
 * Meant to be used with GSAT_WaitTargetDataUsingActor instead of the default WaitTargetData AbilityTask as the default
 * one will destroy the TargetActor.
 */
UCLASS()
class GASSHOOTER_API AGSGATA_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
    AGSGATA_Trace();

	// Base targeting spread (degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float BaseSpread;

	// Aiming spread modifier
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float AimingSpreadMod;

	// Continuous targeting: spread increment
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float TargetingSpreadIncrement;

	// Continuous targeting: max increment
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float TargetingSpreadMax;

	// Current spread from continuous targeting
	float CurrentTargetingSpread;

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

	// Maximum hit results to return per trace. 0 just returns the trace end point.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	int32 MaxHitResultsPerTrace;

	// Number of traces to perform at one time. Single bullet weapons like rilfes will only do one trace.
	// Multi-bullet weapons like shotguns can do multiple traces. Not intended to be used with PersistentHits.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	int32 NumberOfTraces;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bIgnoreBlockingHits;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bTraceFromPlayerViewPoint;

	// HitResults will persist until Confirmation/Cancellation or until a new HitResult takes its place
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bUsePersistentHitResults;

	UFUNCTION(BlueprintCallable)
	virtual void ResetSpread();

	virtual float GetCurrentSpread() const;

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	virtual void SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetDestroyOnConfirmation(bool bInDestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	// Traces as normal, but will manually filter all hit actors
	virtual void LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	virtual void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch = false);

	virtual bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	virtual void StopTargeting();

protected:
	// Trace End point, useful for debug drawing
	FVector CurrentTraceEnd;
	
	TArray<TWeakObjectPtr<AGameplayAbilityWorldReticle>> ReticleActors;
	TArray<FHitResult> PersistentHitResults;

	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitResults) const;
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);

	virtual void DoTrace(TArray<FHitResult>& HitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params) PURE_VIRTUAL(AGSGATA_Trace, return;);
	virtual void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType, float Duration = 2.0f) PURE_VIRTUAL(AGSGATA_Trace, return;);

	virtual AGameplayAbilityWorldReticle* SpawnReticleActor(FVector Location, FRotator Rotation);
	virtual void DestroyReticleActors();
};
