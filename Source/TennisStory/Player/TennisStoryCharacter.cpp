
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
#include "Player/Components/DistanceIndicatorComponent.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/Abilities/SwingAbility.h"
#include "Gameplay/Abilities/VolleyAbility.h"
#include "Net/UnrealNetwork.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#endif

ATennisStoryCharacter::FOnPlayerSpawnedEvent ATennisStoryCharacter::PlayerSpawnedEvent;

const FName ATennisStoryCharacter::BallAttachBone = TEXT("hand_l");

ATennisStoryCharacter::ATennisStoryCharacter()
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

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	BallStrikingComp = CreateDefaultSubobject<UBallStrikingComponent>(TEXT("BallStrikingComp"));

	BallAimingSplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));

	StrikeZone = CreateDefaultSubobject<UBoxComponent>(TEXT("StrikeZone"));
	StrikeZone->SetupAttachment(RootComponent);
	StrikeZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	StrikeZone->SetCollisionProfileName(TEXT("TennisRacquet"));
	StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StrikeZoneLocation_Forehand = CreateDefaultSubobject<USceneComponent>(TEXT("Forehand Strike Zone Location"));
	StrikeZoneLocation_Forehand->SetupAttachment(RootComponent);

	StrikeZoneLocation_Forehand_High = CreateDefaultSubobject<USceneComponent>(TEXT("Forehand High Strike Zone Location"));
	StrikeZoneLocation_Forehand_High->SetupAttachment(RootComponent);
	
	StrikeZoneLocation_Backhand = CreateDefaultSubobject<USceneComponent>(TEXT("Backhand Strike Zone Location"));
	StrikeZoneLocation_Backhand->SetupAttachment(RootComponent);
	
	StrikeZoneLocation_Backhand_High = CreateDefaultSubobject<USceneComponent>(TEXT("Backhand High Strike Zone Location"));
	StrikeZoneLocation_Backhand_High->SetupAttachment(RootComponent);

	DistanceIndicatorRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DistanceIndicatorRingMesh"));
	DistanceIndicatorRing->SetupAttachment(RootComponent);
	DistanceIndicatorRing->SetCollisionProfileName(TEXT("NoCollision"));
	DistanceIndicatorRing->SetHiddenInGame(true);

	DistanceIndicatorComp = CreateDefaultSubobject<UDistanceIndicatorComponent>(TEXT("DistanceIndicatorComp"));
	
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

	DistanceIndicatorComp->VisualComp = DistanceIndicatorRing;

	OnCharacterMovementUpdated.AddDynamic(this, &ATennisStoryCharacter::HandleCharacterMovementUpdated);
}

void ATennisStoryCharacter::Tick(float DeltaSeconds)
{
	//HACK(achester): This is a hack to try and fix a strange issue where SetActorRotation was not correctly working on the autonomous proxy character
	if (Role == ROLE_AutonomousProxy && !GetActorRotation().Quaternion().Equals(ServerDesiredRotation))
	{
		SetActorRotation(ServerDesiredRotation);
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

void ATennisStoryCharacter::EnablePlayerTargeting(ETargetingContext TargetingContext)
{
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (TargetActor && GameState && Controller)
	{
		TargetActor->ShowTargetOnCourt(GameState->GetCourtToAimAtForCharacter(this), IsLocallyControlled(), TargetingContext);
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
}

void ATennisStoryCharacter::CacheCourtAimVector(FVector AimVector)
{
	CachedAimVector = AimVector;
	CachedAimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
}

void ATennisStoryCharacter::PositionStrikeZone(EStrokeType StrokeType)
{
	FVector RelativeLocation = GetStrikeZoneLocationForStroke(StrokeType);

	StrikeZone->SetRelativeLocation(RelativeLocation);
}

FVector ATennisStoryCharacter::GetStrikeZoneLocationForStroke(EStrokeType StrokeType) const
{
	switch (StrokeType)
	{
		case EStrokeType::Backhand:
		{
			return StrikeZoneLocation_Backhand->GetRelativeTransform().GetLocation();
		}
		case EStrokeType::Backhand_High:
		{
			return StrikeZoneLocation_Backhand_High->GetRelativeTransform().GetLocation();
		}
		default:
		case EStrokeType::Forehand:
		{
			return StrikeZoneLocation_Forehand->GetRelativeTransform().GetLocation();
		}
		case EStrokeType::Forehand_High:
		{
			return StrikeZoneLocation_Forehand_High->GetRelativeTransform().GetLocation();
		}
	}
}

void ATennisStoryCharacter::StartDistanceVisualizationToBall()
{
	/*ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	TWeakObjectPtr<ATennisBall> TennisBall = (GameState) ? GameState->GetTennisBall() : nullptr;
	if (TennisBall.IsValid())
	{
		DistanceIndicatorComp->StartVisualizingDistance(TennisBall);
	}*/
}

void ATennisStoryCharacter::StopDistanceVisualization()
{
	DistanceIndicatorComp->StopVisualizingDistance();
}

void ATennisStoryCharacter::Multicast_EnterServiceState_Implementation()
{
	AbilitySystemComp->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Player.State.Service")));
}

void ATennisStoryCharacter::Multicast_ExitServiceState_Implementation()
{
	AbilitySystemComp->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Player.State.Service")));
}

void ATennisStoryCharacter::Multicast_LockAbilities_Implementation()
{
	AbilitySystemComp->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Player.State.AbilitiesLocked")));
}

void ATennisStoryCharacter::Multicast_UnlockAbilities_Implementation()
{
	AbilitySystemComp->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Player.State.AbilitiesLocked")));
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

bool ATennisStoryCharacter::DoesSwingAbilityHavePermissionToActivate(const UGameplayAbility* AskingAbility)
{
	EGroundStrokeAbility AuthorizedGroundstrokeAbility = EGroundStrokeAbility::Swing;

	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (GameState)
	{
		TWeakObjectPtr<AHalfCourt> Court = GameState->GetCourtForCharacter(this);
		bool bIsInFrontCourt = Court.IsValid() && Court->IsLocationInFrontHalfOfCourt(GetActorLocation());

		if (bIsInFrontCourt)
		{
			AuthorizedGroundstrokeAbility = EGroundStrokeAbility::Volley;
		}
	}
	
	switch (AuthorizedGroundstrokeAbility)
	{
		case EGroundStrokeAbility::Swing:
		{
			return AskingAbility->IsA<USwingAbility>();
		}
		case EGroundStrokeAbility::Volley:
		{
			return AskingAbility->IsA<UVolleyAbility>();
		}
	}

	return false;
}

bool ATennisStoryCharacter::ShouldPerformForehand(ATennisBall* TennisBall)
{
	FVector BallDirection = TennisBall->GetCurrentDirection();
	float DistanceToBall = FVector::Dist(TennisBall->GetActorLocation(), GetActorLocation());

	FVector ProjectedBallLocation = TennisBall->GetActorLocation() + BallDirection * DistanceToBall;

	FVector DirToBallProjection = ProjectedBallLocation - GetActorLocation();
	float DotProd = FVector::DotProduct(DirToBallProjection.GetSafeNormal(), GetAimRightVector());
	
	return DotProd >= 0.f;
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
	PlayerInputComponent->BindAxis("MoveForward", this, &ATennisStoryCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATennisStoryCharacter::MoveRight);

	PlayerInputComponent->BindAxis("MoveTargetForwardSimple", this, &ATennisStoryCharacter::AddTargetSimpleForwardInput);
	PlayerInputComponent->BindAxis("MoveTargetRightSimple", this, &ATennisStoryCharacter::AddTargetSimpleRightInput);

	PlayerInputComponent->BindAxis("MoveTargetForwardPrecise", this, &ATennisStoryCharacter::AddTargetPreciseForwardInput);
	PlayerInputComponent->BindAxis("MoveTargetRightPrecise", this, &ATennisStoryCharacter::AddTargetPreciseRightInput);
	
	PlayerInputComponent->BindAxis("MouseAimForward", this, &ATennisStoryCharacter::AddMouseAimForwardInput);
	PlayerInputComponent->BindAxis("MouseAimRight", this, &ATennisStoryCharacter::AddMouseAimRightInput);

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
