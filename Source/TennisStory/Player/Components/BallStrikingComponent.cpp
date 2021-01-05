// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Gameplay/Abilities/BallStrikingAbility.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "TennisStoryGameMode.h"
#include "TennisStoryGameState.h"
#include <../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h>
#include "Net/UnrealNetwork.h"
#include "Gameplay/Ball/GlobalBallVelocityModifier.h"

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bBallStrikingAllowed = false;
}

void UBallStrikingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBallStrikingComponent, ServerDesiredShotType);
}

void UBallStrikingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ATennisStoryCharacter>(GetOwner());
	OwnerRacquet = (OwnerChar) ? OwnerChar->RacquetActor : nullptr;
	OwnerTarget = (OwnerChar) ? OwnerChar->TargetActor : nullptr;
	
	checkf((OwnerRacquet || !OwnerTarget), TEXT("UBallStrikingComponent::BeginPlay - Some required owner pointer not valid!"))
}

void UBallStrikingComponent::AllowBallStriking()
{
	if (bBallStrikingAllowed)
	{
		return;
	}

	//OwnerChar CAN be null when joining a session, as the animations (and notifies) can be replicated before BeginPlay has been called
	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			bBallStrikingAllowed = true;
		}
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	if (!bBallStrikingAllowed)
	{
		return;
	}

	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			bBallStrikingAllowed = false;
		}
	}
}

void UBallStrikingComponent::SetCurrentBallStrikingAbility(UGameplayAbility* AbilityPtr)
{
	ensureMsgf(!AbilityPtr || (AbilityPtr && AbilityPtr->GetClass()->ImplementsInterface(UBallStrikingAbility::StaticClass())), TEXT("BallStriking ability did not implement BallStrikingAbility interface!"));

	CurrentBallstrikingAbility = AbilityPtr;
}

bool UBallStrikingComponent::ShouldWaitForTimingForgiveness(float AnimDelay)
{
	ATennisStoryGameState* TSGameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisBall* TennisBall = (TSGameState) ? TSGameState->GetTennisBall().Get() : nullptr;

	if (TennisBall && TennisBall->BallMovementComp && TennisBall->GetSplineComponent().IsValid())
	{
		//We only want to worry about the ball when it's following a path
		if (TennisBall->BallMovementComp->GetBallMovementState() != EBallMovementState::FollowingPath)
		{
			return false;
		}

		const FBallTrajectoryData& TrajData = TennisBall->BallMovementComp->GetCurrentTrajectoryData();

		FVector CurrentLocation = TennisBall->GetActorLocation();
		FVector BounceLocation = TrajData.TrajectoryPoints[TrajData.BounceLocationIndex].Location;
		FVector EndLocation = TrajData.TrajectoryEndLocation;
		FVector StrikeZoneLocation = TennisBall->GetSplineComponent()->FindLocationClosestToWorldLocation(OwnerChar->GetStrikeZone()->GetComponentLocation(), ESplineCoordinateSpace::World);

		FVector ToCurrentLocation = CurrentLocation - StrikeZoneLocation;
		FVector ToBounceLocation = BounceLocation - StrikeZoneLocation;
		FVector ToEndLocation = EndLocation - StrikeZoneLocation;

		//Since we've fixed our lateral velocity for ball movement, don't consider the Z axis at all
		ToCurrentLocation.Z = 0.f;
		ToBounceLocation.Z = 0.f;
		ToEndLocation.Z = 0.f;

		float DotProd = FVector::DotProduct(ToCurrentLocation.GetSafeNormal(), ToEndLocation.GetSafeNormal());
		//We expect the dot product to be negative here, indicating that the strike zone location is between the current and end locations
		if (DotProd >= 0)
		{
			return false;
		}

		//If the dot product is positive here, it indicates the bounce location is in the same direction as the current location, and therefore we'll need to consider the bounce speed
		bool bWillBounce = (FVector::DotProduct(ToCurrentLocation.GetSafeNormal(), ToBounceLocation.GetSafeNormal()) > 0);

		float DistanceToCurrent = ToCurrentLocation.Size2D();
		float DistanceToBounce = (bWillBounce) ? ToBounceLocation.Size2D() : 0.f;

		//Subtract out bounce distance from total distance if needed
		if (bWillBounce)
		{
			DistanceToCurrent -= DistanceToBounce;
		}

		float TravelDuration = DistanceToCurrent / TrajData.ModifiedVelocity + DistanceToBounce / TrajData.ModifiedBounceVelocity;
		float ForgivenessDuration = TravelDuration - AnimDelay;

		if (ForgivenessDuration > 0.f && ForgivenessDuration <= ForgivenessThreshold)
		{
			FTimerHandle ForgivenessHandle;
			GetWorld()->GetTimerManager().SetTimer(ForgivenessHandle, this, &UBallStrikingComponent::EndTimingForgiveness, ForgivenessDuration, false);

			return true;
		}
	}

	return false;
}

void UBallStrikingComponent::HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//NOTE(achester): This handler is only bound by the Authority so a remote client should never be executing this code

	ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	if (OwnerTarget && TennisBall && TennisBall->GetCurrentBallState() != ETennisBallState::ServiceState)
	{
		if (TennisBall->LastPlayerToHit.IsValid() && TennisBall->LastPlayerToHit == OwnerChar)
		{
			return;
		}

		if (OwnerChar->HasAuthority())
		{
			ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();

			checkf(GameMode, TEXT("UBallStrikingComponent::HandleRacquetOverlapBegin - GameMode was null"))

			GameMode->DetermineHitLegality(OwnerChar);
		}

		IBallStrikingAbility* BallStrikingAbility = Cast<IBallStrikingAbility>(CurrentBallstrikingAbility);

		ensureMsgf(BallStrikingAbility, TEXT("Invalid ball striking ability object - no provided ball speed or trajectory"));
		
		if (OwnerChar->HasAuthority())
		{
			TennisBall->LastPlayerToHit = OwnerChar;
			TennisBall->bWasLastHitAServe = false;

			if (HitFX)
			{
				TennisBall->Multicast_SpawnHitParticleEffect(HitFX, TennisBall->GetActorLocation());
			}

			int ShotQuality = BallStrikingAbility->GetShotQuality();

			int SFXIndex = (ShotQuality >= 0 && ShotQuality < OrderedHitSFX.Num()) ? ShotQuality : 0;
			if (OrderedHitSFX.Num() > 0)
			{
				OwnerChar->Multicast_PlaySound(OrderedHitSFX[SFXIndex], TennisBall->GetActorLocation());
			}
		}

		ensureMsgf(CurrentShotSourceTag != FGameplayTag::EmptyTag, TEXT("No ShotSourceTag available"));
		ensureMsgf(CurrentFallbackShotTypeTag != FGameplayTag::EmptyTag, TEXT("No FallbackShotTypeTag available"));

		FTrajectoryParams TrajParams = UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(CurrentShotSourceTag, CurrentShotContextTags, DesiredShotTypeTag, CurrentFallbackShotTypeTag);

		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams, TennisBall->GetActorLocation(), OwnerTarget->GetActorLocation(), TennisBall);

		TArray<float> MultiplicativeVelocityModifiers = { BallStrikingAbility->GetSpeedMultiplier() };
		UGlobalBallVelocityModifier::CalculateGlobalVelocityModifiers(TrajectoryData, MultiplicativeVelocityModifiers);

		TrajectoryData.ApplyVelocityModifiers(MultiplicativeVelocityModifiers);

		TennisBall->Multicast_FollowPath(TrajectoryData, EBoundsContext::FullCourt, OwnerChar);

		BallHitEvent.Broadcast();
	}
}

void UBallStrikingComponent::OnRep_ServerDesiredShotType()
{
	if (DesiredShotTypeTag != ServerDesiredShotType)
	{
		SetDesiredShotTypeTag(ServerDesiredShotType);
	}
}
