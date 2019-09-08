// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

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

		TargetActor = GetWorld()->SpawnActor<APlayerTargetActor>(TargetActorClass, GetActorTransform(), SpawnParams);
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
		if (TargetActor)
		{
			//Make Character look at target
			FVector TargetLocation = TargetActor->GetActorLocation();
			FVector CharacterLocation = GetActorLocation();
			FVector TranslationVector = TargetLocation - CharacterLocation;
			TranslationVector = FVector(TranslationVector.X, TranslationVector.Y, 0.0f);

			FQuat OrientationQuat = TranslationVector.ToOrientationQuat();

			SetActorRotation(OrientationQuat);
		}

		ATennisBall* TennisBall = GameMode->GetTennisBall();
		if (TennisBall)
		{
			//Set upper body rotation to be facing the ball
			FVector BallLocation = TennisBall->GetActorLocation();
			FVector CharacterLocation = GetActorLocation();
			FVector TranslationVector = BallLocation - CharacterLocation;
			TranslationVector = FVector(TranslationVector.X, TranslationVector.Y, 0.0f);

			FVector TransformedVector = CachedUpperBodyRootBoneTransform.InverseTransformVector(TranslationVector);

			//UpperBodyRotation = TransformedVector.ToOrientationRotator();
			UpperBodyRotation = TranslationVector.ToOrientationRotator();
		}
	}
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
