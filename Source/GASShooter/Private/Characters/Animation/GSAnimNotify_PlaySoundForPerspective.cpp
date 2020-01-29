// Copyright 2020 Dan Kestranek.


#include "Characters/Animation/GSAnimNotify_PlaySoundForPerspective.h"
#include "Animation/AnimSequenceBase.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UGSAnimNotify_PlaySoundForPerspective::UGSAnimNotify_PlaySoundForPerspective()
{
	bPlayForFirstPersonPerspective = true;
}

void UGSAnimNotify_PlaySoundForPerspective::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// Don't call Super to avoid call back in to blueprints
	USoundBase* SoundToPlay = Sound;

	if (SoundToPlay)
	{
		if (SoundToPlay->IsLooping())
		{
			UE_LOG(LogAudio, Warning, TEXT("PlaySound notify: Anim %s tried to spawn infinitely looping sound asset %s. Spawning suppressed."), *GetNameSafe(Animation), *GetNameSafe(SoundToPlay));
			return;
		}

		AGSHeroCharacter* OwningHero = Cast<AGSHeroCharacter>(MeshComp->GetOwner());

		if (!OwningHero)
		{
			// We're in the Montage Editor Window, always play the sounds

			if (bFollow)
			{
				UGameplayStatics::SpawnSoundAttached(SoundToPlay, MeshComp, AttachName, FVector(ForceInit), EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), SoundToPlay, MeshComp->GetComponentLocation(), VolumeMultiplier, PitchMultiplier);
			}

			return;
		}

		// We won't replicate first person animations, but Server (Host as listen server) will still play them.
		// Avoid playing first person sounds for listen server.
		if (bPlayForFirstPersonPerspective && !OwningHero->IsLocallyControlled() && !OwningHero->IsPlayerControlled())
		{
			return;
		}

		// Always play third person sounds unless autonomous client is in first person. Play first person sounds if the OwningHero is in first person.
		// Simulated clients won't play first person animations so they will never play first person sounds.
		// Listen server won't play simulated first person sounds because of the check above.
		if ((!bPlayForFirstPersonPerspective && !OwningHero->IsLocallyControlled() && !OwningHero->IsPlayerControlled())
			|| bPlayForFirstPersonPerspective == OwningHero->IsInFirstPersonPerspective())
		{
			if (bFollow)
			{
				UGameplayStatics::SpawnSoundAttached(SoundToPlay, MeshComp, AttachName, FVector(ForceInit), EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), SoundToPlay, MeshComp->GetComponentLocation(), VolumeMultiplier, PitchMultiplier);
			}
		}
	}
}
