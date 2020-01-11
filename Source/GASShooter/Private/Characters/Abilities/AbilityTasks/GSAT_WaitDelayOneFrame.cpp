// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitDelayOneFrame.h"
#include "TimerManager.h"

UGSAT_WaitDelayOneFrame::UGSAT_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGSAT_WaitDelayOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGSAT_WaitDelayOneFrame::OnDelayFinish);
}

UGSAT_WaitDelayOneFrame* UGSAT_WaitDelayOneFrame::WaitDelayOneFrame(UGameplayAbility* OwningAbility)
{
	UGSAT_WaitDelayOneFrame* MyObj = NewAbilityTask<UGSAT_WaitDelayOneFrame>(OwningAbility);
	return MyObj;
}

void UGSAT_WaitDelayOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	EndTask();
}
