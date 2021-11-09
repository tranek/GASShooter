// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/GSGATA_Trace.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "WorldCollision.h"
#include "GSGATA_LineTrace.generated.h"

/**
 * Reusable, configurable line trace TargetActor.
 * Meant to be used with GSAT_WaitTargetDataUsingActor instead of the default WaitTargetData AbilityTask as the default
 * one will destroy the TargetActor.
 */
UCLASS()
class GASSHOOTER_API AGSGATA_LineTrace : public AGSGATA_Trace
{
	GENERATED_BODY()

public:
	AGSGATA_LineTrace();

	/**
	* Configure the TargetActor for use. This TargetActor could be used in multiple abilities and there's no guarantee
	* what state it will be in. You will need to make sure that only one ability is using this TargetActor at a time.
	*
	* @param InStartLocation Location to trace from.
	* @param InAimingTag Optional. Predicted GameplayTag for aiming. Only used if we modify spread while aiming. If used,
	* must set InAimingRemovalTag also.
	* @param InAimingRemovalTag Optional. Predicted GameplayTag for aiming removal. Only used if we modify spread while
	* aiming. If used, must set InAimingTag also.
	* @param InTraceProfile Collision profile to use for tracing.
	* @param InFilter Hit Actors must pass this filter to be returned in the TargetData.
	* @param InReticleClass Reticle that will appear on top of acquired targets. Reticles will be spawned/despawned as targets are acquired/lost.
	* @param InReticleParams Parameters for world reticle. Usage of these parameters is dependent on the reticle.
	* @param bInIgnoreBlockingHits Ignore blocking collision hits in the trace. Useful if you want to target through walls.
	* @param bInShouldProduceTargetDataOnServer If set, this TargetActor will produce TargetData on the Server in addition
	* to the client and the client will just send a generic "Confirm" event to the server. If false, the client will send
	* the TargetData to the Server. This is handled by the WaitTargetDataUsingActor AbilityTask.
	* @param bInUsePersistentHitResults Should HitResults persist while targeting? HitResults are cleared on Confirm/Cancel or
	* when new HitResults take their place.
	* @param bInDebug When true, this TargetActor will show debug lines of the trace and hit results.
	* @param bInTraceAffectsAimPitch Does the trace affect the aiming pitch?
	* @param bInTraceFromPlayerViewPoint Should we trace from the player ViewPoint instead of the StartLocation? The
	* TargetData HitResults will still have the StartLocation for the TraceStart. This is useful for FPS where we want
	* to trace from the player ViewPoint but draw the bullet tracer from the weapon muzzle.
	* TODO: AI Controllers should fall back to muzzle location. Not implemented yet.
	* @param bInUseAImingSpreadMod Should we modify spread based on if we're aiming? If true, must set InAimingTag and
	* InAimingRemovalTag.
	* @param InMaxRange Max range for this trace.
	* @param InBaseSpread Base targeting spread in degrees.
	* @param InAimingSpreadMod Optional. Multiplicative modifier to spread if aiming.
	* @param InTargetingSpreadIncrement Amount spread increments from continuous targeting in degrees.
	* @param InTargetingSpreadMax Maximum amount of spread for continuous targeting in degrees.
	* @param InMaxHitResultsPerTrace Max hit results that a trace can return. < 1 just returns the trace end point.
	* @param InNumberOfTraces Number of traces to perform. Intended to be used with BaseSpread for multi-shot weapons
	* like shotguns. Not intended to be used with PersistentHitsResults. If using PersistentHitResults, NumberOfTraces is
	* hardcoded to 1. You will need to add support for this in your project if you need it.
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
		UPARAM(DisplayName = "Use Persistent Hit Results") bool bInUsePersistentHitResults = false,
		UPARAM(DisplayName = "Debug") bool bInDebug = false,
		UPARAM(DisplayName = "Trace Affects Aim Pitch") bool bInTraceAffectsAimPitch = true,
		UPARAM(DisplayName = "Trace From Player ViewPoint") bool bInTraceFromPlayerViewPoint = false,
		UPARAM(DisplayName = "Use Aiming Spread Mod") bool bInUseAimingSpreadMod = false,
		UPARAM(DisplayName = "Max Range") float InMaxRange = 999999.0f,
		UPARAM(DisplayName = "Base Targeting Spread") float InBaseSpread = 0.0f,
		UPARAM(DisplayName = "Aiming Spread Mod") float InAimingSpreadMod = 0.0f,
		UPARAM(DisplayName = "Targeting Spread Increment") float InTargetingSpreadIncrement = 0.0f,
		UPARAM(DisplayName = "Targeting Spread Max") float InTargetingSpreadMax = 0.0f,
		UPARAM(DisplayName = "Max Hit Results Per Trace") int32 InMaxHitResultsPerTrace = 1,
		UPARAM(DisplayName = "Number of Traces") int32 InNumberOfTraces = 1
	);

protected:

	virtual void DoTrace(TArray<FHitResult>& HitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params) override;
	virtual void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType, float Duration = 2.0f) override;

#if ENABLE_DRAW_DEBUG
	// Util for drawing result of multi line trace from KismetTraceUtils.h
	void DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType, bool bHit, const TArray<FHitResult>& OutHits, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);
#endif // ENABLE_DRAW_DEBUG
};
