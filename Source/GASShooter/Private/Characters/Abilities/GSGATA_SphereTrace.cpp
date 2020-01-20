// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGATA_SphereTrace.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AGSGATA_SphereTrace::AGSGATA_SphereTrace()
{
	bDestroyOnConfirmation = false;
	MaxTargetResults = 1;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	MaxRange = 999999.0f;
	bTraceAffectsAimPitch = true;
}

void AGSGATA_SphereTrace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AGSGATA_SphereTrace::SetShouldProduceTargetDataOnServer(bool InShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = InShouldProduceTargetDataOnServer;
}

void AGSGATA_SphereTrace::SetDestroyOnConfirmation(bool DestroyOnConfirmation)
{
	bDestroyOnConfirmation = DestroyOnConfirmation;
}

void AGSGATA_SphereTrace::StartTargeting(UGameplayAbility* Ability)
{
	SetActorTickEnabled(true);
	Hits = TArray<FHitResult>();
	Super::StartTargeting(Ability);
}

void AGSGATA_SphereTrace::CancelTargeting()
{
	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	UAbilitySystemComponent* ASC = (ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	if (ASC)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey()).Remove(GenericCancelHandle);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s"), *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
}

void AGSGATA_SphereTrace::BeginPlay()
{
	Super::BeginPlay();

	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in the WaitTargetDataUsingActor task.
	// Ideally we would have a StopTargeting() function on the TargetActor base class.
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately disable tick.
	SetActorTickEnabled(false);
}

void AGSGATA_SphereTrace::Tick(float DeltaSeconds)
{
	// TODO draw the line and a sphere at every hit

	// very temp - do a mostly hardcoded trace from the source actor
	if (SourceActor)
	{
		PerformTrace(SourceActor);
		
		FVector EndPoint = Hits.Last().Component.IsValid() ? Hits.Last().ImpactPoint : Hits.Last().TraceEnd;

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			DrawDebugLine(GetWorld(), SourceActor->GetActorLocation(), EndPoint, FColor::Green, false);
			DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Yellow, false);

			for (int32 i = 0; i < Hits.Num() - 1; i++)
			{
				DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Green, false);
			}			
		}
#endif // ENABLE_DRAW_DEBUG

		SetActorLocationAndRotation(EndPoint, SourceActor->GetActorRotation());
	}
}

void AGSGATA_SphereTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		bDebug = false;
		PerformTrace(SourceActor);
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(Hits);
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

void AGSGATA_SphereTrace::LineTraceWithFilter(FHitResult& OutHitResult, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		const FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.Actor.IsValid() || FilterHandle.FilterPassesForActor(Hit.Actor))
		{
			OutHitResult = Hit;
			OutHitResult.bBlockingHit = true; // treat it as a blocking hit
			return;
		}
	}
}

void AGSGATA_SphereTrace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch) const
{
	if (!OwningAbility) // Server and launching client only
	{
		return;
	}

	APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTraceWithFilter(HitResult, InSourceActor->GetWorld(), Filter, ViewStart, ViewEnd, TraceProfile.Name, Params);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// Convert to angles and use original pitch
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool AGSGATA_SphereTrace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)		//If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;						//Subtracting instead of adding will get the other intersection point
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);		//Cam aim point clipped to range sphere
			return true;
		}
	}
	return false;
}

FGameplayAbilityTargetDataHandle AGSGATA_SphereTrace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		/** Note: These are cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
		FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
		ReturnData->HitResult = HitResults[i];
		ReturnDataHandle.Add(ReturnData);
	}

	return ReturnDataHandle;
}

TArray<FHitResult> AGSGATA_SphereTrace::PerformTrace(AActor* InSourceActor)
{
	//TODO
	// Check Existing Hits if they're still valid and visible. This will require AI Perception Component.
	// GetKnownPerceivedActors()
	// If we haven't reached max number of targets in Hits, perform trace
	// Sphere trace until blocking hit
	// If overlap hits are visible, add to Hits up to max number

	return TArray<FHitResult>();
}
