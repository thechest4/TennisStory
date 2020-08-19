// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugPawn.h"
#include "Components/HighlightableStaticMeshComponent.h"
#include "Debug/TrajectoryTestActor.h"

#define TraceType_MouseSelect TraceTypeQuery3

ADebugPawn::ADebugPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentHighlightMesh = nullptr;
	CurrentMouseDragType = EMouseDragType::None;
	bLockCurrentHighlightMesh = false;

	RootComponent = CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
}

void ADebugPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Pressed, this, &ADebugPawn::StartLeftMouseDrag);
	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Released, this, &ADebugPawn::StopLeftMouseDrag);

	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Pressed, this, &ADebugPawn::StartRightMouseDrag);
	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Released, this, &ADebugPawn::StopRightMouseDrag);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ADebugPawn::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ADebugPawn::MoveRight);
}

void ADebugPawn::StartLeftMouseDrag()
{
	if (CurrentMouseDragType == EMouseDragType::None)
	{
		if (CurrentHighlightMesh)
		{
			CurrentMouseDragType = EMouseDragType::HighlightXY;
			bLockCurrentHighlightMesh = true;
		}
		else
		{
			CurrentMouseDragType = EMouseDragType::Camera;
		}
	}
}

void ADebugPawn::StopLeftMouseDrag()
{
	if (CurrentMouseDragType == EMouseDragType::HighlightXY || CurrentMouseDragType == EMouseDragType::Camera)
	{
		CurrentMouseDragType = EMouseDragType::None;
		bLockCurrentHighlightMesh = false;
	}
}

void ADebugPawn::StartRightMouseDrag()
{
	if (CurrentMouseDragType == EMouseDragType::None)
	{
		if (CurrentHighlightMesh)
		{
			CurrentMouseDragType = EMouseDragType::HighlightZ;
			bLockCurrentHighlightMesh = true;
		}
		else
		{
			CurrentMouseDragType = EMouseDragType::Camera;
		}
	}
}

void ADebugPawn::StopRightMouseDrag()
{
	if (CurrentMouseDragType == EMouseDragType::HighlightZ || CurrentMouseDragType == EMouseDragType::Camera)
	{
		CurrentMouseDragType = EMouseDragType::None;
		bLockCurrentHighlightMesh = false;
	}
}

void ADebugPawn::MoveForward(float Value)
{
	AddActorWorldOffset(GetActorForwardVector() * Value * CameraMoveSpeed * GetWorld()->GetDeltaSeconds());
}

void ADebugPawn::MoveRight(float Value)
{
	AddActorWorldOffset(GetActorRightVector() * Value * CameraMoveSpeed * GetWorld()->GetDeltaSeconds());
}

void ADebugPawn::BeginPlay()
{
	Super::BeginPlay();

	PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = true;
	}
}

void ADebugPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult OutHit;
	PC->GetHitResultUnderCursorByChannel(TraceType_MouseSelect, false, OutHit);

	if (!bLockCurrentHighlightMesh)
	{
		UHighlightableStaticMeshComponent* HighlightMesh = Cast<UHighlightableStaticMeshComponent>(OutHit.Component);
		if (HighlightMesh)
		{
			if (CurrentHighlightMesh && HighlightMesh != CurrentHighlightMesh)
			{
				CurrentHighlightMesh->EndHighlight();
			}

			if (HighlightMesh != CurrentHighlightMesh)
			{
				CurrentHighlightMesh = HighlightMesh;
				CurrentHighlightMesh->StartHighlight(HighlightMat);
			}
		}
		else if (CurrentHighlightMesh)
		{
			CurrentHighlightMesh->EndHighlight();
			CurrentHighlightMesh = nullptr;
		}
	}

	float MouseXDelta, MouseYDelta;
	PC->GetInputMouseDelta(MouseXDelta, MouseYDelta);

	if ((CurrentMouseDragType == EMouseDragType::HighlightXY || CurrentMouseDragType == EMouseDragType::HighlightZ) && CurrentHighlightMesh)
	{
		ATrajectoryTestActor* TrajectoryTestActor = Cast<ATrajectoryTestActor>(CurrentHighlightMesh->GetOwner());
		if (TrajectoryTestActor)
		{
			FVector CamToHighlight = CurrentHighlightMesh->GetComponentLocation() - GetActorLocation();

			FVector MouseWorldPos, MouseDir;
			PC->DeprojectMousePositionToWorld(MouseWorldPos, MouseDir);

			FVector MouseToHighlight = CurrentHighlightMesh->GetComponentLocation() - MouseWorldPos;

			//Calculate desired mouse location using trig
			float CosAngle = FVector::DotProduct(MouseToHighlight.GetSafeNormal(), MouseDir);
			float DistanceAlongDir = MouseToHighlight.Size() * CosAngle;
			FVector NewLoc = MouseWorldPos + MouseDir * DistanceAlongDir;

			//Represent new location as an offset vector
			FVector Offset = NewLoc - CurrentHighlightMesh->GetComponentLocation();

			if (CurrentMouseDragType == EMouseDragType::HighlightXY)
			{
				//Get the scalar projection onto the right vector of the camera
				float RightTranslationLength = FVector::DotProduct(Offset, GetActorRightVector());
				FVector RightTranslation = GetActorRightVector() * RightTranslationLength;
			
				//Get the scalar projection onto the up vector of the camera but apply it to the forward vector using camera right and world up
				FVector ForwardDir = FVector::CrossProduct(GetActorRightVector(), FVector::UpVector);
				float ForwardTranslationLength = FVector::DotProduct(Offset, GetActorUpVector());
				FVector ForwardTranslation = ForwardDir * ForwardTranslationLength;

				CurrentHighlightMesh->AddWorldOffset(RightTranslation);
				CurrentHighlightMesh->AddWorldOffset(ForwardTranslation);
			}
			else if (CurrentMouseDragType == EMouseDragType::HighlightZ)
			{
				//Get the scalar projection onto the world up vector
				float UpTranslationLength = FVector::DotProduct(Offset, FVector::UpVector);
				FVector UpTranslation = FVector::UpVector * UpTranslationLength;

				CurrentHighlightMesh->AddWorldOffset(UpTranslation);
			}
		}
	}
	else if (CurrentMouseDragType == EMouseDragType::Camera)
	{
		float XDelta, YDelta;
		PC->GetInputMouseDelta(XDelta, YDelta);

		FRotator CurrentRotation = GetActorRotation();
		float NewPitch = FMath::Clamp(CurrentRotation.Pitch + YDelta * CameraRotationSpeed, -85.f, 85.f);
		float NewYaw = CurrentRotation.Yaw + XDelta * CameraRotationSpeed;

		SetActorRotation(FRotator(NewPitch, NewYaw, CurrentRotation.Roll));
	}
}

