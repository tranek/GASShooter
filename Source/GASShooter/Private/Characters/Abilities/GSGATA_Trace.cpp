// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGATA_Trace.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameplayAbilitySpec.h"

AGSGATA_Trace::AGSGATA_Trace()
{
	bDestroyOnConfirmation = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	MaxHitResults = 1;
	bIgnoreBlockingHits = false;
	bTraceAffectsAimPitch = true;
	bTraceFromPlayerViewPoint = false;
	MaxRange = 999999.0f;
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
	bUsePersistentHitResults = false;
}

void AGSGATA_Trace::ResetSpread()
{
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
}

float AGSGATA_Trace::GetCurrentSpread() const
{
	float FinalSpread = BaseSpread + CurrentTargetingSpread;

	if (bUseAimingSpreadMod && AimingTag.IsValid() && AimingRemovalTag.IsValid())
	{
		UAbilitySystemComponent* ASC = OwningAbility->GetCurrentActorInfo()->AbilitySystemComponent.Get();
		if (ASC && (ASC->GetTagCount(AimingTag) > ASC->GetTagCount(AimingRemovalTag)))
		{
			FinalSpread *= AimingSpreadMod;
		}
	}

	return FinalSpread;
}

void AGSGATA_Trace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AGSGATA_Trace::SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
}

void AGSGATA_Trace::SetDestroyOnConfirmation(bool bInDestroyOnConfirmation)
{
	bDestroyOnConfirmation = bDestroyOnConfirmation;
}

void AGSGATA_Trace::StartTargeting(UGameplayAbility* Ability)
{
	// Don't call to Super because we can have more than one Reticle

	SetActorTickEnabled(true);

	OwningAbility = Ability;
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	// This is a lazy way of emptying and repopulating the ReticleActors.
	// We could come up with a solution that reuses them.
	DestroyReticleActors();

	if (ReticleClass)
	{
		for (int32 i = 0; i < MaxHitResults; i++)
		{
			SpawnReticleActor(GetActorLocation(), GetActorRotation());
		}
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForDuration, 2.0f);
		}
#endif
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::CancelTargeting()
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

	SetActorTickEnabled(false);

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::BeginPlay()
{
	Super::BeginPlay();

	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in StopTargeting().
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately StopTargeting().
	SetActorTickEnabled(false);
}

void AGSGATA_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();

	Super::EndPlay(EndPlayReason);
}

void AGSGATA_Trace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<FHitResult> HitResults;
	if (bDebug || bUsePersistentHitResults)
	{
		// Only need to trace on Tick if we're showing debug or if we use persistent hit results, otherwise we just use the confirmation trace
		HitResults = PerformTrace(SourceActor);
	}

#if ENABLE_DRAW_DEBUG
	if (SourceActor && bDebug)
	{
		ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForOneFrame);
	}
#endif
}

void AGSGATA_Trace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	TArray<FHitResult> FilteredHitResults;

	// Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.Actor.IsValid() || FilterHandle.FilterPassesForActor(Hit.Actor))
		{
			Hit.TraceStart = TraceStart;
			Hit.TraceEnd = End;

			FilteredHitResults.Add(Hit);
		}
	}

	OutHitResults = FilteredHitResults;

	return;
}

void AGSGATA_Trace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch)
{
	if (!OwningAbility) // Server and launching client only
	{
		return;
	}

	// Default values in case of AI Controller
	FVector ViewStart = TraceStart;
	FRotator ViewRot = StartLocation.GetTargetingTransform().GetRotation().Rotator();

	if (MasterPC)
	{
		MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);
	}

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	// Use first hit
	TArray<FHitResult> HitResults;
	LineTraceWithFilter(HitResults, InSourceActor->GetWorld(), Filter, ViewStart, ViewEnd, TraceProfile.Name, Params);

	CurrentTargetingSpread = FMath::Min(TargetingSpreadMax, CurrentTargetingSpread + TargetingSpreadIncrement);

	const bool bUseTraceResult = HitResults.Num() > 0 && (FVector::DistSquared(TraceStart, HitResults[0].Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResults[0].Location : ViewEnd;

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

	const float CurrentSpread = GetCurrentSpread();

	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AdjustedAimDir, ConeHalfAngle, ConeHalfAngle);

	OutTraceEnd = TraceStart + (ShootDir * MaxRange);
}

bool AGSGATA_Trace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
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

void AGSGATA_Trace::StopTargeting()
{
	SetActorTickEnabled(false);

	DestroyReticleActors();

	// Clear added callbacks
	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();

	if (GenericDelegateBoundASC)
	{
		GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::ConfirmTargeting);
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::CancelTargeting);
	}
}

FGameplayAbilityTargetDataHandle AGSGATA_Trace::MakeTargetData(const TArray<FHitResult>& HitResults) const
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

TArray<FHitResult> AGSGATA_Trace::PerformTrace(AActor* InSourceActor)
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

	CurrentTraceEnd = TraceEnd;

	TArray<FHitResult> ReturnHitResults;
	DoTrace(ReturnHitResults, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

	if (bUsePersistentHitResults)
	{
		// Clear any blocking hit results, invalid Actors, or actors out of range
		// Check for visibility if we add AIPerceptionComponent in the future
		for (int32 i = PersistentHitResults.Num() - 1; i >= 0; i--)
		{
			FHitResult& HitResult = PersistentHitResults[i];

			if (HitResult.bBlockingHit || !HitResult.Actor.IsValid() || FVector::DistSquared(TraceStart, HitResult.Actor.Get()->GetActorLocation()) > (MaxRange * MaxRange))
			{
				PersistentHitResults.RemoveAt(i);
			}
		}
	}

	for (int32 i = ReturnHitResults.Num() - 1; i >= 0; i--)
	{
		if (MaxHitResults >= 0 && i + 1 > MaxHitResults)
		{
			// Trim to MaxHitResults
			ReturnHitResults.RemoveAt(i);
			continue;
		}

		FHitResult& HitResult = ReturnHitResults[i];

		if (bUsePersistentHitResults)
		{
			// This is looping backwards so that further objects from player are added first to the queue.
			// This results in closer actors taking precedence as the further actors will get bumped out of the TArray.
			if (HitResult.Actor.IsValid() && (!HitResult.bBlockingHit || PersistentHitResults.Num() < 1))
			{
				bool bActorAlreadyInPersistentHits = false;

				// Make sure PersistentHitResults doesn't have this hit actor already
				for (int32 j = 0; j < PersistentHitResults.Num(); j++)
				{
					FHitResult& PersistentHitResult = PersistentHitResults[j];
					
					if (PersistentHitResult.Actor.Get() == HitResult.Actor.Get())
					{
						bActorAlreadyInPersistentHits = true;
						break;
					}
				}

				if (bActorAlreadyInPersistentHits)
				{
					continue;
				}

				if (PersistentHitResults.Num() >= MaxHitResults)
				{
					// Treat PersistentHitResults like a queue, remove first element
					PersistentHitResults.RemoveAt(0);
				}

				PersistentHitResults.Add(HitResult);
			}
		}
		else
		{
			// ReticleActors for PersistentHitResults are handled later
			if (i < ReticleActors.Num())
			{
				if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[i].Get())
				{
					const bool bHitActor = HitResult.Actor != nullptr;

					if (bHitActor && !HitResult.bBlockingHit)
					{
						LocalReticleActor->SetActorHiddenInGame(false);

						const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.Actor->GetActorLocation() : HitResult.Location;

						LocalReticleActor->SetActorLocation(ReticleLocation);
						LocalReticleActor->SetIsTargetAnActor(bHitActor);
					}
					else
					{
						LocalReticleActor->SetActorHiddenInGame(true);
					}
				}
			}
		}
	}

	if (!bUsePersistentHitResults)
	{
		if (ReturnHitResults.Num() < ReticleActors.Num())
		{
			// We have less hit results than ReticleActors, hide the extra ones
			for (int32 i = ReturnHitResults.Num(); i < ReticleActors.Num(); i++)
			{
				if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[i].Get())
				{
					LocalReticleActor->SetIsTargetAnActor(false);
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
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

		if (bUsePersistentHitResults && PersistentHitResults.Num() < 1)
		{
			PersistentHitResults.Add(HitResult);
		}
	}

	if (bUsePersistentHitResults && MaxHitResults > 0)
	{
		// Handle ReticleActors
		for (int32 i = 0; i < PersistentHitResults.Num(); i++)
		{
			FHitResult& HitResult = PersistentHitResults[i];

			if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[i].Get())
			{
				const bool bHitActor = HitResult.Actor != nullptr;

				if (bHitActor && !HitResult.bBlockingHit)
				{
					LocalReticleActor->SetActorHiddenInGame(false);

					const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.Actor->GetActorLocation() : HitResult.Location;

					LocalReticleActor->SetActorLocation(ReticleLocation);
					LocalReticleActor->SetIsTargetAnActor(bHitActor);
				}
				else
				{
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		if (PersistentHitResults.Num() < ReticleActors.Num())
		{
			// We have less hit results than ReticleActors, hide the extra ones
			for (int32 i = PersistentHitResults.Num(); i < ReticleActors.Num(); i++)
			{
				if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[i].Get())
				{
					LocalReticleActor->SetIsTargetAnActor(false);
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		return PersistentHitResults;
	}

	return ReturnHitResults;
}

AGameplayAbilityWorldReticle* AGSGATA_Trace::SpawnReticleActor(FVector Location, FRotator Rotation)
{
	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation);
		if (SpawnedReticleActor)
		{
			SpawnedReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
			SpawnedReticleActor->SetActorHiddenInGame(true);
			ReticleActors.Add(SpawnedReticleActor);

			// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
			// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
			// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
			// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
			// actors and there may be other targeting types that want to replicate the same reticle actor class).
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}

			return SpawnedReticleActor;
		}
	}

	return nullptr;
}

void AGSGATA_Trace::DestroyReticleActors()
{
	for (int32 i = ReticleActors.Num() - 1; i >= 0; i--)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i].Get()->Destroy();
		}
	}

	ReticleActors.Empty();
}
