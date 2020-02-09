// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGATA_LineTrace.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameplayAbilitySpec.h"
#include "Kismet/KismetSystemLibrary.h"

AGSGATA_LineTrace::AGSGATA_LineTrace()
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
	WeaponSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	FiringSpreadIncrement = 0.0f;
	FiringSpreadMax = 0.0f;
	CurrentFiringSpread = 0.0f;
}

void AGSGATA_LineTrace::ResetSpread()
{
	bUseAimingSpreadMod = false;
	WeaponSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	FiringSpreadIncrement = 0.0f;
	FiringSpreadMax = 0.0f;
	CurrentFiringSpread = 0.0f;
}

float AGSGATA_LineTrace::GetCurrentSpread() const
{
	float FinalSpread = WeaponSpread + CurrentFiringSpread;

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
	float InWeaponSpread,
	float InAimingSpreadMod,
	float InFiringSpreadIncrement,
	float InFiringSpreadMax,
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
	WeaponSpread = InWeaponSpread;
	AimingSpreadMod = InAimingSpreadMod;
	FiringSpreadIncrement = InFiringSpreadIncrement;
	FiringSpreadMax = InFiringSpreadMax;
	MaxHitResults = InMaxHitResults;
}

void AGSGATA_LineTrace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AGSGATA_LineTrace::SetShouldProduceTargetDataOnServer(bool InShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = InShouldProduceTargetDataOnServer;
}

void AGSGATA_LineTrace::SetDestroyOnConfirmation(bool DestroyOnConfirmation)
{
	bDestroyOnConfirmation = DestroyOnConfirmation;
}

void AGSGATA_LineTrace::StartTargeting(UGameplayAbility* Ability)
{
	SetActorTickEnabled(true);

	// Don't call to Super because we can have more than one Reticle

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

void AGSGATA_LineTrace::CancelTargeting()
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
}

void AGSGATA_LineTrace::BeginPlay()
{
	Super::BeginPlay();

	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in the WaitTargetDataUsingActor task.
	// Ideally we would have a StopTargeting() function on the TargetActor base class.
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately disable tick.
	SetActorTickEnabled(false);
}

void AGSGATA_LineTrace::Tick(float DeltaSeconds)
{
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

void AGSGATA_LineTrace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();

	Super::EndPlay(EndPlayReason);
}

void AGSGATA_LineTrace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
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

void AGSGATA_LineTrace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch)
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

	CurrentFiringSpread = FMath::Min(FiringSpreadMax, CurrentFiringSpread + FiringSpreadIncrement);

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
	
	UE_LOG(LogTemp, Log, TEXT("%s Current Spread: %f"), TEXT(__FUNCTION__), CurrentSpread);

	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AdjustedAimDir, ConeHalfAngle, ConeHalfAngle);

	OutTraceEnd = TraceStart + (ShootDir * MaxRange);
}

bool AGSGATA_LineTrace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
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

FGameplayAbilityTargetDataHandle AGSGATA_LineTrace::MakeTargetData(const TArray<FHitResult>& HitResults) const
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

void AGSGATA_LineTrace::SpawnReticleActor(FVector Location, FRotator Rotation)
{
	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation);
		if (SpawnedReticleActor)
		{
			SpawnedReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
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
		}
	}
}

void AGSGATA_LineTrace::DestroyReticleActors()
{
	for (int32 i = ReticleActors.Num() - 1; i >= 0; i++)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i].Get()->Destroy();
		}
	}

	ReticleActors.Empty();
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
