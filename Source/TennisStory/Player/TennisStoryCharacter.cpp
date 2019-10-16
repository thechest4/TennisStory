
#include "TennisStoryCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TennisStoryGameMode.h"
#include "Player/PlayerTargetActor.h"
#include "Player/Components/BallStrikingComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/TennisBall.h"
#include "Gameplay/HalfCourt.h"

ATennisStoryCharacter::ATennisStoryCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	BallStrikingComp = CreateDefaultSubobject<UBallStrikingComponent>(TEXT("BallStrikingComp"));
}

void ATennisStoryCharacter::BeginPlay()
{
	Super::BeginPlay();

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
	}

	TArray<FName> BoneNames;
	GetMesh()->GetBoneNames(BoneNames);

	for (int i = 0; i < BoneNames.Num(); i++)
	{
		if (BoneNames[i] == UpperBodyRootBone)
		{
			CachedUpperBodyRootBoneTransform = GetMesh()->GetBoneTransform(i);
			break;
		}
	}

	if (AbilitySystemComp && HasAuthority())
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);

		for (FGrantedAbilityInfo AbilityInfo : AbilitiesToGive)
		{
			AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityInfo.AbilityClass.GetDefaultObject(), (int)AbilityInfo.AbilityInput, 0));
		}
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

void ATennisStoryCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	if (GameMode)
	{
		AActor* LookAtTarget = TargetActor;

		TWeakObjectPtr<ATennisBall> TennisBall = GameMode->GetTennisBall();
		if (TennisBall.IsValid())
		{
			LookAtTarget = TennisBall.Get();
		}

		if (LookAtTarget)
		{
			//Make Character look at target
			FVector TargetLocation = LookAtTarget->GetActorLocation();
			FVector CharacterLocation = GetActorLocation();
			FVector TranslationVector = TargetLocation - CharacterLocation;

			const static bool bApplyRotationOffset = true;
			if (bApplyRotationOffset)
			{
				TranslationVector = TranslationVector.RotateAngleAxis(-30.0f, FVector::UpVector);
			}

			FQuat OrientationQuat = TranslationVector.GetSafeNormal2D().ToOrientationQuat();

			SetActorRotation(OrientationQuat);
		}
	}
}

void ATennisStoryCharacter::EnablePlayerTargeting()
{
	if (TargetActor)
	{
		TargetActor->ShowTargetOnCourt(GetCourtToAimAt());
	}

	//TODO(achester): set up a list of character movement modifications so that multiple can be applied/removed safely
	//also put some thought into how to best store stats like move speed while swinging and such.
	CachedMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeedWhileSwinging;
}

void ATennisStoryCharacter::FreezePlayerTarget()
{
	if (TargetActor)
	{
		TargetActor->DisableTargetMovement();
	}
}

void ATennisStoryCharacter::DisablePlayerTargeting()
{
	if (TargetActor)
	{
		TargetActor->HideTarget();
	}

	GetCharacterMovement()->MaxWalkSpeed = CachedMaxWalkSpeed;
}

void ATennisStoryCharacter::CacheCourtAimVector(FVector AimVector)
{
	CachedAimVector = AimVector;
	CachedAimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
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

TWeakObjectPtr<class AHalfCourt> ATennisStoryCharacter::GetCourtToAimAt()
{
	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	if (GameMode)
	{
		TArray<TWeakObjectPtr<AHalfCourt>> AvailableCourts = GameMode->GetAllCourts();

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
