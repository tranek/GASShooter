// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_MoveSceneCompRelLocation.generated.h"

class UCurveFloat;
class UCurveVector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoveSceneCompRelLocationDelegate);

/**
 * Moves a SceneComponent (usually a mesh component) over time. This is not a simulated task so it doesn't run on all clients.
 * This is only suitable for cosmetic changes for the local player.
 */
UCLASS()
class GASSHOOTER_API UGSAT_MoveSceneCompRelLocation : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FMoveSceneCompRelLocationDelegate OnFinishMove;

	/** Move the scene component to the specified location, using the vector curve (range 0 - 1) if specified, otherwise the float curve (range 0 - 1) or fallback to linear interpolation */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_MoveSceneCompRelLocation* MoveSceneComponentRelativeLocation(UGameplayAbility* OwningAbility, FName TaskInstanceName, USceneComponent* SceneComponent, FVector Location, float Duration, UCurveFloat* OptionalInterpolationCurve, UCurveVector* OptionalVectorInterpolationCurve);

	virtual void Activate() override;

	/** Tick function for this task, if bTickingTask == true */
	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool AbilityIsEnding) override;

protected:
	bool bIsFinished;

	FVector StartLocation;

	FVector TargetLocation;

	float DurationOfMovement;

	float TimeMoveStarted;

	float TimeMoveWillEnd;

	UPROPERTY()
	USceneComponent* Component;

	UPROPERTY()
	UCurveFloat* LerpCurve;

	UPROPERTY()
	UCurveVector* LerpCurveVector;
};
