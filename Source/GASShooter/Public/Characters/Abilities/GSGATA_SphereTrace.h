// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "CollisionQueryParams.h"
#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"
#include "WorldCollision.h"
#include "GSGATA_SphereTrace.generated.h"

/**
 * Reusable sphere trace that can return multiple overlapping actors. Stops tracing at the first blocking hit.
 */
UCLASS()
class GASSHOOTER_API AGSGATA_SphereTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	AGSGATA_SphereTrace();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	float MaxRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, meta = (ExposeOnSpawn = true), Category = Trace)
	FCollisionProfileName TraceProfile;

	// Does the trace affect the aiming pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	bool bTraceAffectsAimPitch;

	// Maximum number of targets that can be returned on confirmation
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	int32 MaxTargetResults;

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetShouldProduceTargetDataOnServer(bool InShouldProduceTargetDataOnServer);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetDestroyOnConfirmation(bool DestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void ConfirmTargetingAndContinue() override;

	/** Traces as normal, but will manually filter all hit actors */
	static void LineTraceWithFilter(FHitResult& OutHitResult, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

protected:
	FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitActors) const;

	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);

	TArray<FHitResult> Hits;
};
