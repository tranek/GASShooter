// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGATA_LineTrace.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AGSGATA_LineTrace::AGSGATA_LineTrace()
{
}

void AGSGATA_LineTrace::Configure(
	const FGameplayAbilityTargetingLocationInfo& InStartLocation,
	FGameplayTag InAimingTag,
	FGameplayTag InAimingRemovalTag,
	FCollisionProfileName InTraceProfile,
	FGameplayTargetDataFilterHandle InFilter,
	TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
	FWorldReticleParameters InReticleParams,
	bool bInIgnoreBlockingHits,
	bool bInShouldProduceTargetDataOnServer,
	bool bInDebug,
	bool bInTraceAffectsAimPitch,
	bool bInTraceFromPlayerViewPoint,
	bool bInUseAimingSpreadMod,
	float InMaxRange,
	float InBaseSpread,
	float InAimingSpreadMod,
	float InTargetingSpreadIncrement,
	float InTargetingSpreadMax,
	int32 InMaxHitResults)
{
	StartLocation = InStartLocation;
	AimingTag = InAimingTag;
	AimingRemovalTag = InAimingRemovalTag;
	TraceProfile = InTraceProfile;
	Filter = InFilter;
	ReticleClass = InReticleClass;
	ReticleParams = InReticleParams;
	bIgnoreBlockingHits = bInIgnoreBlockingHits;
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
	bDebug = bInDebug;
	bTraceAffectsAimPitch = bInTraceAffectsAimPitch;
	bTraceFromPlayerViewPoint = bInTraceFromPlayerViewPoint;
	bUseAimingSpreadMod = bInUseAimingSpreadMod;
	MaxRange = InMaxRange;
	BaseSpread = InBaseSpread;
	AimingSpreadMod = InAimingSpreadMod;
	TargetingSpreadIncrement = InTargetingSpreadIncrement;
	TargetingSpreadMax = InTargetingSpreadMax;
	MaxHitResults = InMaxHitResults;
}

void AGSGATA_LineTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);

#if ENABLE_DRAW_DEBUG
		if (bDebug && HitResults.Num() > 0)
		{
			FVector ViewStart = HitResults[0].TraceStart;
			FRotator ViewRot;
			if (MasterPC)
			{
				MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);
			}

			DrawDebugLineTraceMulti(GetWorld(), bTraceFromPlayerViewPoint ? ViewStart : HitResults[0].TraceStart,
				HitResults[0].TraceEnd, EDrawDebugTrace::ForDuration, true, HitResults, FLinearColor::Green, FLinearColor::Red, 2.0f);
		}
#endif
	}
}

void AGSGATA_LineTrace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);

#if ENABLE_DRAW_DEBUG
		if (bDebug && HitResults.Num() > 0)
		{
			DrawDebugLineTraceMulti(GetWorld(), HitResults[0].TraceStart, HitResults[0].TraceEnd, EDrawDebugTrace::ForOneFrame, true, HitResults, FLinearColor::Green, FLinearColor::Red, 2.0f);
		}
#endif
	}
}

TArray<FHitResult> AGSGATA_LineTrace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGSGATA_LineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bIgnoreBlocks = bIgnoreBlockingHits;

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	if (MasterPC)
	{
		FVector ViewStart;
		FRotator ViewRot;
		MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);

		TraceStart = bTraceFromPlayerViewPoint ? ViewStart : TraceStart;
	}

	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);		//Effective on server and launching client only


	// ------------------------------------------------------

	SetActorLocationAndRotation(TraceEnd, SourceActor->GetActorRotation());

	TArray<FHitResult> ReturnHitResults;
	LineTraceWithFilter(ReturnHitResults, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

	for (int32 i = ReturnHitResults.Num() - 1; i >= 0 ; i--)
	{
		if (MaxHitResults >= 0 && i + 1 > MaxHitResults)
		{
			// Trim to MaxHitResults
			ReturnHitResults.RemoveAt(i);
			continue;
		}

		FHitResult& HitResult = ReturnHitResults[i];

		if (MaxHitResults > 0)
		{
			if (i < ReticleActors.Num())
			{
				if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[i].Get())
				{
					const bool bHitActor = HitResult.Actor != nullptr;
					const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.Actor->GetActorLocation() : HitResult.Location;

					LocalReticleActor->SetActorLocation(ReticleLocation);
					LocalReticleActor->SetIsTargetAnActor(bHitActor);
				}
			}
		}
		else
		{
			// Infinite MaxHitResults, spawn a new ReticleActor for each hit result
			SpawnReticleActor(GetActorLocation(), GetActorRotation());
		}
	}

	if (ReturnHitResults.Num() < 1)
	{
		// If there were no hits, add a default HitResult at the end of the trace
		FHitResult HitResult;
		// Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
		HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
		HitResult.TraceEnd = TraceEnd;
		HitResult.Location = TraceEnd;
		HitResult.ImpactPoint = TraceEnd;
		ReturnHitResults.Add(HitResult);
	}

	return ReturnHitResults;
}

#if ENABLE_DRAW_DEBUG
// Copied from KismetTraceUtils.cpp
void AGSGATA_LineTrace::DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType, bool bHit, const TArray<FHitResult>& OutHits, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		// @fixme, draw line with thickness = 2.f?
		if (bHit && OutHits.Last().bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			FVector const BlockingHitPoint = OutHits.Last().ImpactPoint;
			::DrawDebugLine(World, Start, BlockingHitPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			::DrawDebugLine(World, BlockingHitPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			::DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}

		// draw hits
		for (int32 HitIdx = 0; HitIdx < OutHits.Num(); ++HitIdx)
		{
			FHitResult const& Hit = OutHits[HitIdx];
			::DrawDebugPoint(World, Hit.ImpactPoint, 16.0f, (Hit.bBlockingHit ? TraceColor.ToFColor(true) : TraceHitColor.ToFColor(true)), bPersistent, LifeTime);
		}
	}
}
#endif // ENABLE_DRAW_DEBUG
