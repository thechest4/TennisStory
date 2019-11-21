
#include "TennisStoryCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TennisStoryGameState.h"
#include "Player/PlayerTargetActor.h"
#include "Player/Components/BallStrikingComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/HalfCourt.h"
#include "Net/UnrealNetwork.h"

ATennisStoryCharacter::FOnPlayerSpawnedEvent ATennisStoryCharacter::PlayerSpawnedEvent;

ATennisStoryCharacter::ATennisStoryCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	BallStrikingComp = CreateDefaultSubobject<UBallStrikingComponent>(TEXT("BallStrikingComp"));

	BallAimingSplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));

	StrikeZone = CreateDefaultSubobject<UBoxComponent>(TEXT("StrikeZone"));
	StrikeZone->SetupAttachment(RootComponent);
	StrikeZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	StrikeZone->SetCollisionProfileName(TEXT("TennisRacquet"));
	StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATennisStoryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisStoryCharacter, CachedAimVector);
	DOREPLIFETIME(ATennisStoryCharacter, CachedAimRightVector);
	DOREPLIFETIME(ATennisStoryCharacter, bIsCharging);
}

void ATennisStoryCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			for (FGrantedAbilityInfo AbilityInfo : AbilitiesToGive)
			{
				AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityInfo.AbilityClass.GetDefaultObject(), (int)AbilityInfo.AbilityInput, 0));
			}
		}
	}

	ATennisStoryCharacter::PlayerSpawnedEvent.Broadcast(this);
}

void ATennisStoryCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (RacquetClass)
	{
		FActorSpawnParameters SpawnParams = FActorSpawnParameters();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		RacquetActor = GetWorld()->SpawnActor<ATennisRacquet>(RacquetClass, GetActorTransform(), SpawnParams);
		RacquetActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RacquetAttachBone);
	}

	if (TargetActorClass)
	{
		FActorSpawnParameters SpawnParams = FActorSpawnParameters();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector ActorLocationOnGround = GetActorLocation();
		ActorLocationOnGround.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		TargetActor = GetWorld()->SpawnActor<APlayerTargetActor>(TargetActorClass, ActorLocationOnGround + GetActorForwardVector() * 800.0f, GetActorRotation(), SpawnParams);
		TargetActor->SetOwner(this);
	}
}

void ATennisStoryCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComp)
	{
		AbilitySystemComp->RefreshAbilityActorInfo();
	}
}

void ATennisStoryCharacter::EnablePlayerTargeting()
{
	if (TargetActor)
	{
		TargetActor->ShowTargetOnCourt(GetCourtToAimAt(), IsLocallyControlled());

		//NOTE(achester): Since the target hasn't had a chance to move, this will just force the target into the center snap point.  
		//Basically a fallback in case another position isn't committed
		if (IsLocallyControlled())
		{
			Server_CommitTargetPosition(TargetActor->GetActorLocation());
		}
	}

	//TODO(achester): set up a list of character movement modifications so that multiple can be applied/removed safely
	//also put some thought into how to best store stats like move speed while swinging and such.
	if (HasAuthority())
	{
		bIsCharging = true;
		OnRep_IsCharging();
	}
}

void ATennisStoryCharacter::FreezePlayerTarget()
{
	if (TargetActor)
	{
		TargetActor->DisableTargetMovement();
		
		if (IsLocallyControlled())
		{
			Server_CommitTargetPosition(TargetActor->GetActorLocation());
		}
	}
}

void ATennisStoryCharacter::DisablePlayerTargeting()
{
	if (TargetActor)
	{
		TargetActor->HideTarget();
	}

	if (HasAuthority())
	{
		bIsCharging = false;
		OnRep_IsCharging();
	}
}

void ATennisStoryCharacter::CacheCourtAimVector(FVector AimVector)
{
	CachedAimVector = AimVector;
	CachedAimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
}

float ATennisStoryCharacter::GetStrikeZoneSize()
{
	return StrikeZone->GetScaledBoxExtent().X;
}

void ATennisStoryCharacter::PositionStrikeZone(FVector NewRelativeLocation)
{
	StrikeZone->SetRelativeLocation(NewRelativeLocation);
}

void ATennisStoryCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATennisStoryCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATennisStoryCharacter::MoveRight);

	PlayerInputComponent->BindAxis("MoveTargetForward", this, &ATennisStoryCharacter::MoveTargetForward);
	PlayerInputComponent->BindAxis("MoveTargetRight", this, &ATennisStoryCharacter::MoveTargetRight);

	AbilitySystemComp->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "EAbilityInput"));
}

void ATennisStoryCharacter::OnRep_IsCharging()
{
	if (bIsCharging)
	{
		CachedMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeedWhileSwinging;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = CachedMaxWalkSpeed;
	}
}

void ATennisStoryCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATennisStoryCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATennisStoryCharacter::MoveTargetForward(float Value)
{
	if (TargetActor && Controller && Value != 0.0f)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		TargetActor->AddInputVector(Direction, Value);
	}
}

void ATennisStoryCharacter::MoveTargetRight(float Value)
{
	if (TargetActor && Controller && Value != 0.0f)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		TargetActor->AddInputVector(Direction, Value);
	}
}

bool ATennisStoryCharacter::Server_CommitTargetPosition_Validate(FVector WorldLocation)
{
	return TargetActor;
}

void ATennisStoryCharacter::Server_CommitTargetPosition_Implementation(FVector WorldLocation)
{
	if (!IsLocallyControlled())
	{	
		TargetActor->SetActorLocation(WorldLocation);
	}

	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	TWeakObjectPtr<ATennisBall> TennisBall = (GameState) ? GameState->GetTennisBall() : nullptr;
	if (!TennisBall.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryCharacter::Server_CommitTargetPosition_Implementation - ATennisStoryGameState::GetTennisBall returned null!"));
		return;
	}

	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(BallStrikingComp->GetTrajectoryCurve(), TennisBall->GetActorLocation(), TargetActor->GetActorLocation(), 200.f, 500.f);
	BallStrikingComp->SetTrajectory(TrajectoryData);
	UBallAimingFunctionLibrary::DebugVisualizeSplineComp(BallAimingSplineComp);

	Multicast_ReceiveBallTrajectory(TrajectoryData);
}

void ATennisStoryCharacter::Multicast_ReceiveBallTrajectory_Implementation(FBallTrajectoryData TrajectoryData)
{
	//Only clients need to receive Trajectory
	if (HasAuthority())
	{
		return;
	}
	
	BallStrikingComp->SetTrajectory(TrajectoryData);
	UBallAimingFunctionLibrary::DebugVisualizeSplineComp(BallAimingSplineComp);
}

TWeakObjectPtr<class AHalfCourt> ATennisStoryCharacter::GetCourtToAimAt()
{
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (GameState)
	{
		TArray<TWeakObjectPtr<AHalfCourt>> AvailableCourts = GameState->GetAllCourts();

		if (AvailableCourts.Num() == 0)
		{
			return nullptr;
		}
		else if (AvailableCourts.Num() == 1)
		{
			return AvailableCourts[0];
		}
		else
		{
			//The court to aim at will be the one furthest from the character, so just find out whichever one that is
			//TODO(achester): use court registration once that feature exists

			TWeakObjectPtr<AHalfCourt> CurrentFurthestCourt = nullptr;

			for (TWeakObjectPtr<AHalfCourt> Court : AvailableCourts)
			{
				if (!CurrentFurthestCourt.IsValid())
				{
					CurrentFurthestCourt = Court;
				}
				else
				{
					float DistanceToCourt = FVector::Dist(GetActorLocation(), Court->GetActorLocation());
					float DistanceToFurthestCourt = FVector::Dist(GetActorLocation(), CurrentFurthestCourt->GetActorLocation());

					if (DistanceToCourt > DistanceToFurthestCourt)
					{
						CurrentFurthestCourt = Court;
					}
				}
			}

			return CurrentFurthestCourt;
		}
	}

	return nullptr;
}
