
#include "TennisStoryCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TennisStoryGameState.h"
#include "Player/PlayerTargetActor.h"
#include "Player/PlayerMouseTarget.h"
#include "Player/Components/BallStrikingComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/Abilities/SwingAbility.h"
#include "Gameplay/Abilities/VolleyAbility.h"
#include "Gameplay/Abilities/DiveAbility.h"
#include "Net/UnrealNetwork.h"
#include <AbilitySystemBlueprintLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "Components/TrajectoryPreviewComponent.h"
#include <Particles/ParticleSystemComponent.h>
#include "../Gameplay/TrajectoryDataProvider.h"
#include "../Gameplay/Abilities/ForgivingAbilityInterface.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#endif

ATennisStoryCharacter::FOnPlayerSpawnedEvent ATennisStoryCharacter::PlayerSpawnedEvent;

const FName ATennisStoryCharacter::BallAttachBone = TEXT("hand_l");

ATennisStoryCharacter::ATennisStoryCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	bHasBallAttached = false;
	DefaultBaseMovementSpeed = 0.f;
	bIsLocationClamped = false;
	ClampLocation1 = FVector(-1.f, -1.f, -1.f);
	ClampLocation2 = FVector(-1.f, -1.f, -1.f);
	bEnableRotationFix = true;
	LastShotTypeRequestTimestamp = 0.f;
	CurrentSwingStance = ESwingStance::Neutral;

	AbilitySystemComp = CreateOptionalDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	BallStrikingComp = CreateOptionalDefaultSubobject<UBallStrikingComponent>(TEXT("BallStrikingComp"));

	BallAimingSplineComp = CreateOptionalDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));

	TrajectoryPreviewComp = CreateOptionalDefaultSubobject<UTrajectoryPreviewComponent>(TEXT("TrajectoryPreviewComp"));

	StrikeZone = CreateOptionalDefaultSubobject<UBoxComponent>(TEXT("StrikeZone"));
	StrikeZone->SetupAttachment(RootComponent);
	StrikeZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	StrikeZone->SetCollisionProfileName(TEXT("TennisRacquet"));
	StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StrikeZoneLocation_Forehand = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Forehand Strike Zone Location"));
	StrikeZoneLocation_Forehand->SetupAttachment(RootComponent);

	StrikeZoneLocation_Forehand_High = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Forehand High Strike Zone Location"));
	StrikeZoneLocation_Forehand_High->SetupAttachment(RootComponent);
	
	StrikeZoneLocation_Backhand = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Backhand Strike Zone Location"));
	StrikeZoneLocation_Backhand->SetupAttachment(RootComponent);
	
	StrikeZoneLocation_Backhand_High = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Backhand High Strike Zone Location"));
	StrikeZoneLocation_Backhand_High->SetupAttachment(RootComponent);
	
	StrikeZoneLocation_Dive = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("Dive Strike Zone Location"));
	StrikeZoneLocation_Dive->SetupAttachment(RootComponent);
	
#if WITH_EDITORONLY_DATA
	static ConstructorHelpers::FObjectFinder<UTexture2D> TennisRacquetSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/tennis-racket"));
	
	static const float IconEditorScale = 0.5f;

	StrikeZoneIcon_Forehand = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Strike Zone Icon Forehand"));
	if (StrikeZoneIcon_Forehand)
	{
		StrikeZoneIcon_Forehand->SetupAttachment(StrikeZoneLocation_Forehand);
		StrikeZoneIcon_Forehand->SetHiddenInGame(true);
		StrikeZoneIcon_Forehand->SetRelativeLocation(FVector::ZeroVector);
		StrikeZoneIcon_Forehand->SetEditorScale(IconEditorScale);

		if (TennisRacquetSprite.Succeeded())
		{
			StrikeZoneIcon_Forehand->SetSprite(TennisRacquetSprite.Object);
		}
	}

	StrikeZoneIcon_Forehand_High = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Strike Zone Icon Forehand High"));
	if (StrikeZoneIcon_Forehand_High)
	{
		StrikeZoneIcon_Forehand_High->SetupAttachment(StrikeZoneLocation_Forehand_High);
		StrikeZoneIcon_Forehand_High->SetHiddenInGame(true);
		StrikeZoneIcon_Forehand_High->SetRelativeLocation(FVector::ZeroVector);
		StrikeZoneIcon_Forehand_High->SetEditorScale(IconEditorScale);

		if (TennisRacquetSprite.Succeeded())
		{
			StrikeZoneIcon_Forehand_High->SetSprite(TennisRacquetSprite.Object);
		}
	}
	
	StrikeZoneIcon_Backhand = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Strike Zone Icon Backhand"));
	if (StrikeZoneIcon_Backhand)
	{
		StrikeZoneIcon_Backhand->SetupAttachment(StrikeZoneLocation_Backhand);
		StrikeZoneIcon_Backhand->SetHiddenInGame(true);
		StrikeZoneIcon_Backhand->SetRelativeLocation(FVector::ZeroVector);
		StrikeZoneIcon_Backhand->SetEditorScale(IconEditorScale);

		if (TennisRacquetSprite.Succeeded())
		{
			StrikeZoneIcon_Backhand->SetSprite(TennisRacquetSprite.Object);
		}
	}
	
	StrikeZoneIcon_Backhand_High = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Strike Zone Icon Backhand High"));
	if (StrikeZoneIcon_Backhand_High)
	{
		StrikeZoneIcon_Backhand_High->SetupAttachment(StrikeZoneLocation_Backhand_High);
		StrikeZoneIcon_Backhand_High->SetHiddenInGame(true);
		StrikeZoneIcon_Backhand_High->SetRelativeLocation(FVector::ZeroVector);
		StrikeZoneIcon_Backhand_High->SetEditorScale(IconEditorScale);

		if (TennisRacquetSprite.Succeeded())
		{
			StrikeZoneIcon_Backhand_High->SetSprite(TennisRacquetSprite.Object);
		}
	}
	
	StrikeZoneIcon_Dive = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Strike Zone Icon Dive"));
	if (StrikeZoneIcon_Dive)
	{
		StrikeZoneIcon_Dive->SetupAttachment(StrikeZoneLocation_Dive);
		StrikeZoneIcon_Dive->SetHiddenInGame(true);
		StrikeZoneIcon_Dive->SetRelativeLocation(FVector::ZeroVector);
		StrikeZoneIcon_Dive->SetEditorScale(IconEditorScale);

		if (TennisRacquetSprite.Succeeded())
		{
			StrikeZoneIcon_Dive->SetSprite(TennisRacquetSprite.Object);
		}
	}
#endif
}

void ATennisStoryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisStoryCharacter, CachedAimVector);
	DOREPLIFETIME(ATennisStoryCharacter, CachedAimRightVector);
	DOREPLIFETIME(ATennisStoryCharacter, TeamId);
	DOREPLIFETIME(ATennisStoryCharacter, ServerDesiredRotation);
	DOREPLIFETIME(ATennisStoryCharacter, bHasBallAttached);
	DOREPLIFETIME(ATennisStoryCharacter, bIsLocationClamped);
	DOREPLIFETIME(ATennisStoryCharacter, ClampLocation1);
	DOREPLIFETIME(ATennisStoryCharacter, ClampLocation2);
	DOREPLIFETIME(ATennisStoryCharacter, TargetActor);
}

void ATennisStoryCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultBaseMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			for (FGrantedAbilityInfo& AbilityInfo : AbilitiesToGive)
			{
				AbilityInfo.AbilitySpecHandle = AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityInfo.AbilityClass.GetDefaultObject(), 0, static_cast<int>(AbilityInfo.AbilityInput)));
			}
		}
	}

	ATennisStoryCharacter::PlayerSpawnedEvent.Broadcast(this);

	//HACK(achester): This is a hack to try and fix a strange issue where SetActorRotation was not correctly working on the autonomous proxy character
	if (HasAuthority())
	{
		ServerDesiredRotation = GetActorRotation().Quaternion();
	}

	OnCharacterMovementUpdated.AddDynamic(this, &ATennisStoryCharacter::HandleCharacterMovementUpdated);
}

void ATennisStoryCharacter::Tick(float DeltaSeconds)
{
	//HACK(achester): This is a hack to try and fix a strange issue where SetActorRotation was not correctly working on the autonomous proxy character
	if (GetLocalRole() == ROLE_AutonomousProxy && !GetActorRotation().Quaternion().Equals(ServerDesiredRotation) && bEnableRotationFix)
	{
		SetActorRotation(ServerDesiredRotation);
	}

	if (AbilitySystemComp && IsLocallyControlled())
	{
		TArray<FGameplayAbilitySpec> AbilSpecs = AbilitySystemComp->GetActivatableAbilities();
		FGameplayAbilitySpecHandle ActiveAbilityHandle;

		for (int i = 0; i < AbilSpecs.Num(); i++)
		{
			UGameplayAbility* PrimaryInstance = AbilSpecs[i].GetPrimaryInstance();
			ICoreSwingAbility* CoreSwingAbil = (PrimaryInstance) ? Cast<ICoreSwingAbility>(PrimaryInstance) : nullptr;
			if (CoreSwingAbil && PrimaryInstance->IsActive() && !CoreSwingAbil->HasReleased()) //If the active ability has already released we don't want to cancel it; it's already in the process of resolving
			{
				ActiveAbilityHandle = AbilSpecs[i].Handle;
			}
		}

		//We only want to actually call cancel/activate abilities if an ability is already active
		if (ActiveAbilityHandle.IsValid())
		{
			FGameplayTag CoreSwingEventTag;
			FGameplayAbilitySpecHandle DesiredAbilityHandle = GetHandleForAppropriateCoreSwingAbility(CoreSwingEventTag);

			if (DesiredAbilityHandle.IsValid() && ActiveAbilityHandle != DesiredAbilityHandle)
			{
				AbilitySystemComp->CancelAbilityHandle(ActiveAbilityHandle);

				if (CoreSwingEventTag.IsValid())
				{
					FGameplayEventData EventData = FGameplayEventData();
					EventData.InstigatorTags.AddTag(FGameplayTag::RequestGameplayTag(TAG_CORESWING_CONTEXTCHANGE));

					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CoreSwingEventTag, EventData);
				}
			}
		}
	}
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

	if (TargetActorClass && HasAuthority())
	{
		FActorSpawnParameters SpawnParams = FActorSpawnParameters();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector ActorLocationOnGround = GetActorLocation();
		ActorLocationOnGround.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		TargetActor = GetWorld()->SpawnActor<APlayerTargetActor>(TargetActorClass, ActorLocationOnGround + GetActorForwardVector() * 800.0f, GetActorRotation(), SpawnParams);
		TargetActor->SetOwner(this);
	}

	if (TrajectoryPreviewComp)
	{
		TrajectoryPreviewComp->SetBallStrikingCompReference(BallStrikingComp);
	}
}

void ATennisStoryCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComp)
	{
		AbilitySystemComp->RefreshAbilityActorInfo();
	}

	if (HasAuthority() && NewController != nullptr)
	{
		//NOTE(achester): this is kinda weird here since we probably don't want to be caching/replicating the team id, but actually it's only currently used as a quick way to set team colors
		ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
		if (GameState && TeamColorMaterials.Num())
		{
			const FTeamData& TeamData = GameState->GetTeamForPlayer(Cast<ATennisStoryPlayerController>(NewController));
			TeamId = TeamData.TeamId;
			OnRep_TeamId();
		}
	}
}

void ATennisStoryCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RacquetActor)
	{
		RacquetActor->Destroy();
	}

	if (TargetActor)
	{
		TargetActor->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void ATennisStoryCharacter::EnablePlayerTargeting(ETargetingContext TargetingContext, UObject* OverrideTrajSourceObj)
{
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (TargetActor && GameState && Controller)
	{
		TargetActor->ShowTargetOnCourt(GameState->GetCourtToAimAtForCharacter(this), IsLocallyControlled(), TargetingContext);

		if (TrajectoryPreviewComp && IsLocallyControlled())
		{
			TrajectoryPreviewComp->StartShowingTrajectory(BallAimingSplineComp, (OverrideTrajSourceObj) ? OverrideTrajSourceObj : StrikeZone, TargetActor);
		}
	}
}

void ATennisStoryCharacter::FreezePlayerTarget()
{
	if (TargetActor)
	{
		TargetActor->DisableTargetMovement();
	}

	if (MouseTarget)
	{
		MouseTarget->HideTarget();
	}
}

void ATennisStoryCharacter::DisablePlayerTargeting()
{
	if (TargetActor)
	{
		TargetActor->HideTarget();
	}

	if (MouseTarget)
	{
		MouseTarget->HideTarget();
	}

	if (TrajectoryPreviewComp)
	{
		TrajectoryPreviewComp->StopShowingTrajectory();
	}
}

void ATennisStoryCharacter::CacheCourtAimVector(FVector AimVector)
{
	CachedAimVector = AimVector;
	CachedAimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
}

void ATennisStoryCharacter::PositionStrikeZone(EStrikeZoneLocation StrokeType)
{
	FVector RelativeLocation = GetStrikeZoneLocationForStroke(StrokeType);

	StrikeZone->SetRelativeLocation(RelativeLocation);
}

FVector ATennisStoryCharacter::GetStrikeZoneLocationForStroke(EStrikeZoneLocation StrokeType) const
{
	switch (StrokeType)
	{
		case EStrikeZoneLocation::Backhand:
		{
			return StrikeZoneLocation_Backhand->GetRelativeTransform().GetLocation();
		}
		case EStrikeZoneLocation::Backhand_High:
		{
			return StrikeZoneLocation_Backhand_High->GetRelativeTransform().GetLocation();
		}
		default:
		case EStrikeZoneLocation::Forehand:
		{
			return StrikeZoneLocation_Forehand->GetRelativeTransform().GetLocation();
		}
		case EStrikeZoneLocation::Forehand_High:
		{
			return StrikeZoneLocation_Forehand_High->GetRelativeTransform().GetLocation();
		}
		case EStrikeZoneLocation::Dive:
		{
			return StrikeZoneLocation_Dive->GetRelativeTransform().GetLocation();
		}
	}
}

void ATennisStoryCharacter::Multicast_EnterServiceState_Implementation()
{
	AbilitySystemComp->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(STATETAG_SERVICE));
}

void ATennisStoryCharacter::Multicast_ExitServiceState_Implementation()
{
	AbilitySystemComp->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(STATETAG_SERVICE));
}

void ATennisStoryCharacter::Multicast_LockAbilities_Implementation()
{
	AbilitySystemComp->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(STATETAG_ABILITIESLOCKED));
}

void ATennisStoryCharacter::Multicast_UnlockAbilities_Implementation()
{
	AbilitySystemComp->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(STATETAG_ABILITIESLOCKED));
}

void ATennisStoryCharacter::Multicast_SetActorTransform_Implementation(FTransform NewTransform)
{
	SetActorTransform(NewTransform);
}

void ATennisStoryCharacter::AttachBallToPlayer(ATennisBall* TennisBall)
{
	TennisBall->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ATennisStoryCharacter::BallAttachBone);

	bHasBallAttached = true;
}

void ATennisStoryCharacter::DetachBallFromPlayer(ATennisBall* TennisBall)
{
	TennisBall->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	bHasBallAttached = false;
}

void ATennisStoryCharacter::Multicast_LockMovement_Implementation()
{
	GetCharacterMovement()->DisableMovement();
}

void ATennisStoryCharacter::Multicast_UnlockMovement_Implementation()
{
	GetCharacterMovement()->SetDefaultMovementMode();
}

void ATennisStoryCharacter::ClampLocation(FVector MinLocation, FVector MaxLocation)
{
	bIsLocationClamped = true;
	ClampLocation1 = MinLocation;
	ClampLocation2 = MaxLocation;
}

void ATennisStoryCharacter::UnclampLocation()
{
	bIsLocationClamped = false;
}

void ATennisStoryCharacter::ResetPlayStates()
{
	CancelAllAbilities();

	if (BallStrikingComp)
	{
		BallStrikingComp->ResetAllShotTags();
	}
}

void ATennisStoryCharacter::CancelAllAbilities()
{
	for (FGrantedAbilityInfo& AbilityInfo : AbilitiesToGive)
	{
		AbilitySystemComp->CancelAbilityHandle(AbilityInfo.AbilitySpecHandle);
	}
}

void ATennisStoryCharacter::Multicast_PlaySound_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
}

FGameplayAbilitySpecHandle ATennisStoryCharacter::GetHandleForAppropriateCoreSwingAbility(FGameplayTag& OutEventTag)
{
	if (!AbilitySystemComp)
	{
		return FGameplayAbilitySpecHandle();
	}

	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (GameState)
	{
		TWeakObjectPtr<AHalfCourt> Court = GameState->GetCourtForCharacter(this);
		bool bIsInFrontCourt = Court.IsValid() && Court->IsLocationInFrontHalfOfCourt(GetActorLocation());

		//NOTE(achester): using this enum to match type, because FindAbilitySpecFromClass doesn't seem to work when comparing native base class (USwingAbility) to the bp derived class
		ECoreSwingAbility CoreSwingAbilityType = ECoreSwingAbility::Swing;

		if (bIsInFrontCourt)
		{
			CoreSwingAbilityType = ECoreSwingAbility::Volley;
		}

		TArray<FGameplayAbilitySpec> AbilSpecs = AbilitySystemComp->GetActivatableAbilities();

		for (int i = 0; i < AbilSpecs.Num(); i++)
		{
			switch (CoreSwingAbilityType)
			{
				case ECoreSwingAbility::Swing:
				{
					if (AbilSpecs[i].Ability->IsA<USwingAbility>())
					{
						OutEventTag = FGameplayTag::RequestGameplayTag(EVENTTAG_SWING);
						return AbilSpecs[i].Handle;
					}
				}
				case ECoreSwingAbility::Volley:
				{
					if (AbilSpecs[i].Ability->IsA<UVolleyAbility>())
					{
						OutEventTag = FGameplayTag::RequestGameplayTag(EVENTTAG_VOLLEY);
						return AbilSpecs[i].Handle;
					}
				}
			}
		}
	}

	return FGameplayAbilitySpecHandle();
}

ESwingStance ATennisStoryCharacter::CalculateNewSwingStance(ATennisBall* TennisBall)
{
	FVector ProjectedBallLocation;

	//If we have a trajectory, just get the closest location on the spline
	if (TennisBall->GetCurrentMovementState() == EBallMovementState::FollowingPath)
	{
		ProjectedBallLocation = TennisBall->GetSplineComponent()->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	}
	else //Otherwise use some naive projection
	{
		FVector BallDirection = TennisBall->GetCurrentDirection();
		float DistanceToBall = FVector::Dist(TennisBall->GetActorLocation(), GetActorLocation());

		ProjectedBallLocation = TennisBall->GetActorLocation() + BallDirection * DistanceToBall;
	}

	FVector CharacterReferenceLocation = GetActorLocation();
	if (CurrentSwingStance != ESwingStance::Neutral)
	{
		//Distance offset from character center to reduce volatility of stance selection
		const float STANCE_CHANGE_OFFSET = 30.f;

		FVector StanceOffsetVector = GetAimRightVector() * ((CurrentSwingStance == ESwingStance::Forehand) ? -STANCE_CHANGE_OFFSET : STANCE_CHANGE_OFFSET);

		CharacterReferenceLocation += StanceOffsetVector;
	}

	FVector DirToBallProjection = ProjectedBallLocation - CharacterReferenceLocation;
	float DotProd = FVector::DotProduct(DirToBallProjection.GetSafeNormal(), GetAimRightVector());

	return (DotProd >= 0.f) ? ESwingStance::Forehand : ESwingStance::Backhand;
}

void ATennisStoryCharacter::Multicast_ReleaseForgivingAbility_Implementation(FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComp->FindAbilitySpecFromHandle(AbilitySpecHandle);

	if (AbilitySpec && AbilitySpec->GetAbilityInstances().Num())
	{
		for (UGameplayAbility* Instance : AbilitySpec->GetAbilityInstances())
		{
			if (Instance->GetClass()->ImplementsInterface(UForgivingAbilityInterface::StaticClass()))
			{
				IForgivingAbilityInterface* ForgivingAbilityPtr = Cast<IForgivingAbilityInterface>(Instance);
				ForgivingAbilityPtr->ReleaseForgiveness();
			}
		}
	}
}

void ATennisStoryCharacter::Multicast_ModifyBaseSpeed_Implementation(float ModifiedBaseSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = ModifiedBaseSpeed;
	CurrentBaseMovementSpeed = ModifiedBaseSpeed;
}

void ATennisStoryCharacter::Multicast_RestoreBaseSpeed_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultBaseMovementSpeed;
	CurrentBaseMovementSpeed = DefaultBaseMovementSpeed;
}

void ATennisStoryCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis(AXISNAME_MOVEFORWARD, this, &ATennisStoryCharacter::MoveForward);
	PlayerInputComponent->BindAxis(AXISNAME_MOVERIGHT, this, &ATennisStoryCharacter::MoveRight);

	PlayerInputComponent->BindAxis("MoveTargetForwardSimple", this, &ATennisStoryCharacter::AddTargetSimpleForwardInput);
	PlayerInputComponent->BindAxis("MoveTargetRightSimple", this, &ATennisStoryCharacter::AddTargetSimpleRightInput);

	PlayerInputComponent->BindAxis("MoveTargetForwardPrecise", this, &ATennisStoryCharacter::AddTargetPreciseForwardInput);
	PlayerInputComponent->BindAxis("MoveTargetRightPrecise", this, &ATennisStoryCharacter::AddTargetPreciseRightInput);
	
	PlayerInputComponent->BindAxis("MouseAimForward", this, &ATennisStoryCharacter::AddMouseAimForwardInput);
	PlayerInputComponent->BindAxis("MouseAimRight", this, &ATennisStoryCharacter::AddMouseAimRightInput);

	PlayerInputComponent->BindAction("Swing", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::PerformCoreSwing);
	PlayerInputComponent->BindAction("Swing", EInputEvent::IE_Released, this, &ATennisStoryCharacter::ReleaseCoreSwing);

	PlayerInputComponent->BindAction("Dive", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::PerformDive);

	PlayerInputComponent->BindAction("ShotType_Topspin", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::RequestTopspinShot);
	PlayerInputComponent->BindAction("ShotType_Slice", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::RequestSliceShot);
	PlayerInputComponent->BindAction("ShotType_Flat", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::RequestFlatShot);
	PlayerInputComponent->BindAction("ShotType_Lob", EInputEvent::IE_Pressed, this, &ATennisStoryCharacter::RequestLobShot);

	AbilitySystemComp->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "EAbilityInput"));
}

void ATennisStoryCharacter::OnRep_TeamId()
{
	if (TeamId >= 0)
	{
		GetMesh()->SetMaterial(0, TeamColorMaterials[TeamId]);
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

void ATennisStoryCharacter::AddTargetForwardInput(float Value)
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

void ATennisStoryCharacter::AddTargetRightInput(float Value)
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

void ATennisStoryCharacter::AddTargetSimpleForwardInput(float Value)
{
	if (TargetActor && TargetActor->GetTargetingMode() == ETargetingMode::Simple)
	{
		AddTargetForwardInput(Value);
	}
}

void ATennisStoryCharacter::AddTargetSimpleRightInput(float Value)
{
	if (TargetActor && TargetActor->GetTargetingMode() == ETargetingMode::Simple)
	{
		AddTargetRightInput(Value);
	}
}

void ATennisStoryCharacter::AddTargetPreciseForwardInput(float Value)
{
	if (TargetActor && Value != 0.f)
	{
		TargetActor->SetTargetingMode(ETargetingMode::Precise);

		AddTargetForwardInput(Value);
	}
}

void ATennisStoryCharacter::AddTargetPreciseRightInput(float Value)
{
	if (TargetActor && Value != 0.f)
	{
		TargetActor->SetTargetingMode(ETargetingMode::Precise);

		AddTargetRightInput(Value);
	}
}

void ATennisStoryCharacter::AddMouseAimForwardInput(float Value)
{
	if (Value != 0.f && TargetActor && TargetActor->IsCurrentlyVisible())
	{
		if (!MouseTarget)
		{
			SpawnMouseTargetActor();
		}

		ensureMsgf(MouseTarget, TEXT("MouseTarget was null!"));

		if (MouseTarget)
		{
			TargetActor->SetTargetingMode(ETargetingMode::Precise);
			TargetActor->SetMovementTarget(MouseTarget);

			if (!MouseTarget->IsCurrentlyShown())
			{
				MouseTarget->SetActorLocation(TargetActor->GetActorLocation());
				MouseTarget->ShowTarget();
			}

			MouseTarget->AddForwardInput(Value);
		}
	}
}

void ATennisStoryCharacter::AddMouseAimRightInput(float Value)
{
	if (Value != 0.f && TargetActor && TargetActor->IsCurrentlyVisible())
	{
		if (!MouseTarget)
		{
			SpawnMouseTargetActor();
		}

		ensureMsgf(MouseTarget, TEXT("MouseTarget was null!"));

		if (MouseTarget)
		{
			TargetActor->SetTargetingMode(ETargetingMode::Precise);
			TargetActor->SetMovementTarget(MouseTarget);

			if (!MouseTarget->IsCurrentlyShown())
			{
				MouseTarget->SetActorLocation(TargetActor->GetActorLocation());
				MouseTarget->ShowTarget();
			}

			MouseTarget->AddRightInput(Value);
		}
	}
}

void ATennisStoryCharacter::SpawnMouseTargetActor()
{
	if (!MouseTarget && MouseTargetClass)
	{
		MouseTarget = GetWorld()->SpawnActor<APlayerMouseTarget>(MouseTargetClass, FTransform::Identity);
	}
}

void ATennisStoryCharacter::PerformCoreSwing()
{
	//Don't try to activate a core swing ability if we're in service state or currently performing a core swing ability
	if (!AbilitySystemComp->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(STATETAG_SERVICE)) && !GetActiveCoreSwingAbility())
	{
		FGameplayTag CoreSwingEventTag;
		GetHandleForAppropriateCoreSwingAbility(CoreSwingEventTag);

		if (CoreSwingEventTag.IsValid())
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CoreSwingEventTag, FGameplayEventData());
		}
	}
}

void ATennisStoryCharacter::ReleaseCoreSwing()
{
	ICoreSwingAbility* ActiveCoreSwingAbility = GetActiveCoreSwingAbility();
	if (!ActiveCoreSwingAbility)
	{
		return;
	}

	ActiveCoreSwingAbility->ReleaseSwing();

	if (!HasAuthority())
	{
		Server_ReleaseCoreSwing();
	}
}

void ATennisStoryCharacter::Server_ReleaseCoreSwing_Implementation()
{
	ReleaseCoreSwing();
}

bool ATennisStoryCharacter::Server_ReleaseCoreSwing_Validate()
{
	return true;
}

void ATennisStoryCharacter::PerformDive()
{
	//NOTE(achester): this function is only ever called by the owning client since it responds directly to input

	FVector2D DiveInputVector = FVector2D(GetInputAxisValue(AXISNAME_MOVERIGHT), GetInputAxisValue(AXISNAME_MOVEFORWARD));

	//Since we don't have a direct way to pass a payload into ActivateAbility, instead we use the GameplayEventData to send targeting data (which is just our input vector)
	FGameplayEventData DiveEventData = FGameplayEventData();
	DiveEventData.TargetData = FGameplayAbilityTargetDataHandle();

	FDiveAbilityTargetData* DiveTargetData = new FDiveAbilityTargetData();
	DiveTargetData->DiveInputVector = FVector_NetQuantize(DiveInputVector.X, DiveInputVector.Y, 0.f);
	DiveEventData.TargetData.Add(DiveTargetData);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(EVENTTAG_DIVE), DiveEventData);
}

ICoreSwingAbility* ATennisStoryCharacter::GetActiveCoreSwingAbility()
{
	ensure(AbilitySystemComp);

	TArray<FGameplayAbilitySpec> AbilSpecs = AbilitySystemComp->GetActivatableAbilities();
	FGameplayAbilitySpecHandle ActiveAbilityHandle;

	for (int i = 0; i < AbilSpecs.Num(); i++)
	{
		UGameplayAbility* PrimaryInstance = AbilSpecs[i].GetPrimaryInstance();
		ICoreSwingAbility* CoreSwingAbil = (PrimaryInstance) ? Cast<ICoreSwingAbility>(PrimaryInstance) : nullptr;
		if (CoreSwingAbil && PrimaryInstance->IsActive())
		{
			return CoreSwingAbil;
		}
	}

	return nullptr;
}

void ATennisStoryCharacter::ChangeDesiredShotType(FGameplayTag DesiredShotTypeTag)
{
	//If we requested a change to our shot type too recently, ignore
	if (GetWorld()->GetTimeSeconds() - LastShotTypeRequestTimestamp <= ShotTypeRequestThrottle)
	{
		return;
	}

	if (BallStrikingComp)
	{
		BallStrikingComp->SetDesiredShotTypeTag(DesiredShotTypeTag);

		Server_ChangeDesiredShotType(DesiredShotTypeTag);

		LastShotTypeRequestTimestamp = GetWorld()->GetTimeSeconds();

		if (ShotTypeChangeParticle)
		{
			UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAttached(ShotTypeChangeParticle, GetMesh());

			//Get the particle color from the color mapping table
			UDataTable* ShotTypeColorMappingDT = UTrajectoryDataProvider::GetDefaultColorMappingTable();

			if (ShotTypeColorMappingDT)
			{
				for (auto Row : ShotTypeColorMappingDT->GetRowMap())
				{
					FShotTypeColorMapping* ColorMapping = reinterpret_cast<FShotTypeColorMapping*>(Row.Value);

					ensureMsgf(ColorMapping, TEXT("Wrong row struct in Color Mapping DT"));

					if (ColorMapping->ShotTypeTag == DesiredShotTypeTag)
					{
						ParticleComp->SetColorParameter(TEXT("ShotTypeColor"), ColorMapping->ShotColor);
						break;
					}
				}
			}
		}
	}
}

bool ATennisStoryCharacter::Server_ChangeDesiredShotType_Validate(FGameplayTag DesiredShotTypeTag)
{
	return DesiredShotTypeTag.IsValid();
}

void ATennisStoryCharacter::Server_ChangeDesiredShotType_Implementation(FGameplayTag argDesiredShotTypeTag)
{
	if (BallStrikingComp)
	{
		BallStrikingComp->ServerDesiredShotType = BallStrikingComp->DesiredShotTypeTag = argDesiredShotTypeTag;
	}
}

void ATennisStoryCharacter::HandleCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	if (bIsLocationClamped)
	{
		FVector CurrentLocation = GetActorLocation();
		
		float ClampedXCoord = (ClampLocation1.X <= ClampLocation2.X) ? FMath::Clamp(CurrentLocation.X, ClampLocation1.X, ClampLocation2.X) : 
																		   FMath::Clamp(CurrentLocation.X, ClampLocation2.X, ClampLocation1.X);
		float ClampedYCoord = (ClampLocation1.Y <= ClampLocation2.Y) ? FMath::Clamp(CurrentLocation.Y, ClampLocation1.Y, ClampLocation2.Y) : 
																		   FMath::Clamp(CurrentLocation.Y, ClampLocation2.Y, ClampLocation1.Y);

		FVector ClampedLocation = FVector(ClampedXCoord, ClampedYCoord, CurrentLocation.Z);

		if (ClampedLocation != CurrentLocation)
		{
			SetActorLocation(ClampedLocation);
		}
	}
}

const FName ATennisStoryCharacter::AXISNAME_MOVEFORWARD = "MoveForward";
const FName ATennisStoryCharacter::AXISNAME_MOVERIGHT	= "MoveRight";

const FName ATennisStoryCharacter::STATETAG_SERVICE			= TEXT("Player.State.Service");
const FName ATennisStoryCharacter::STATETAG_ABILITIESLOCKED	= TEXT("Player.State.AbilitiesLocked");

const FName ATennisStoryCharacter::EVENTTAG_SWING	= TEXT("Player.Event.Swing");
const FName ATennisStoryCharacter::EVENTTAG_VOLLEY	= TEXT("Player.Event.Volley");
const FName ATennisStoryCharacter::EVENTTAG_SMASH	= TEXT("Player.Event.Smash");
const FName ATennisStoryCharacter::EVENTTAG_DIVE	= TEXT("Player.Event.Dive");

const FName ATennisStoryCharacter::TAG_CORESWING_CONTEXTCHANGE = TEXT("CoreSwing.ContextChange");
