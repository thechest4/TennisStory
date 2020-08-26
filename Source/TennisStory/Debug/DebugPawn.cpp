// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugPawn.h"
#include "Debug/TrajectoryTestActor.h"

#define TraceType_MouseSelect TraceTypeQuery3

ADebugPawn::ADebugPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentCursorMovable = nullptr;
	CurrentMouseDragType = EMouseDragType::None;
	bLockCurrentHighlightMesh = false;

	RootComponent = CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
}

void ADebugPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Pressed, this, &ADebugPawn::StartLeftMouseDrag);
	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Released, this, &ADebugPawn::StopLeftMouseDrag);
	
	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_DoubleClick, this, &ADebugPawn::ShowContextMenu);

	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Pressed, this, &ADebugPawn::StartRightMouseDrag);
	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Released, this, &ADebugPawn::StopRightMouseDrag);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ADebugPawn::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ADebugPawn::MoveRight);
}

void ADebugPawn::StartLeftMouseDrag()
{
	if (CurrentMouseDragType == EMouseDragType::None)
	{
		if (CurrentCursorMovable)
		{
			CurrentMouseDragType = EMouseDragType::HighlightXY;
			bLockCurrentHighlightMesh = true;

			CalculateSelectionOffset();
		}
		else
		{
			CurrentMouseDragType = EMouseDragType::Camera;
			
			HideContextMenu();
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
		if (CurrentCursorMovable)
		{
			CurrentMouseDragType = EMouseDragType::HighlightZ;
			bLockCurrentHighlightMesh = true;

			CalculateSelectionOffset();
		}
		else
		{
			CurrentMouseDragType = EMouseDragType::Camera;
		}

		HideContextMenu();
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

	if (Value)
	{
		HideContextMenu();
	}
}

void ADebugPawn::MoveRight(float Value)
{
	AddActorWorldOffset(GetActorRightVector() * Value * CameraMoveSpeed * GetWorld()->GetDeltaSeconds());
	
	if (Value)
	{
		HideContextMenu();
	}
}

void ADebugPawn::ShowContextMenu()
{
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(CurrentCursorMovable);
	if (CurrentCursorMovable && PrimComp)
	{
		CurrentContextMenuActor = Cast<IHasContextMenu>(PrimComp->GetOwner());
		if (CurrentContextMenuActor)
		{
			CurrentContextMenuActor->ShowContextMenu();
		}
	}
}

void ADebugPawn::HideContextMenu()
{
	if (CurrentContextMenuActor)
	{
		CurrentContextMenuActor->HideContextMenu();
		CurrentContextMenuActor = nullptr;
	}
}

void ADebugPawn::CalculateSelectionOffset()
{
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(CurrentCursorMovable);
	if (CurrentCursorMovable && PrimComp)
	{
		FVector2D MeshScreenPos;
		UGameplayStatics::ProjectWorldToScreen(PC, PrimComp->GetComponentLocation(), MeshScreenPos);

		float MouseX, MouseY;
		PC->GetMousePosition(MouseX, MouseY);
		FVector2D MousePos = FVector2D(MouseX, MouseY);

		CursorSelectionOffset = MeshScreenPos - MousePos;
	}
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
		ICursorMovable* CursorMovable = Cast<ICursorMovable>(OutHit.Component);
		if (CursorMovable)
		{
			if (CurrentCursorMovable && CursorMovable != CurrentCursorMovable)
			{
				CurrentCursorMovable->EndHighlight();
			}

			if (CursorMovable != CurrentCursorMovable)
			{
				CurrentCursorMovable = CursorMovable;
				CurrentCursorMovable->StartHighlight(HighlightMat);
			}
		}
		else if (CurrentCursorMovable)
		{
			CurrentCursorMovable->EndHighlight();
			CurrentCursorMovable = nullptr;
		}
	}

	float MouseXDelta, MouseYDelta;
	PC->GetInputMouseDelta(MouseXDelta, MouseYDelta);
	
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(CurrentCursorMovable);
	if ((CurrentMouseDragType == EMouseDragType::HighlightXY || CurrentMouseDragType == EMouseDragType::HighlightZ) && CurrentCursorMovable && PrimComp)
	{
		FVector CamToHighlight = PrimComp->GetComponentLocation() - GetActorLocation();

		float MouseX, MouseY;
		PC->GetMousePosition(MouseX, MouseY);
		FVector2D MousePos = FVector2D(MouseX, MouseY);

		FVector MouseWorldPos, MouseDir;
		UGameplayStatics::DeprojectScreenToWorld(PC, MousePos + CursorSelectionOffset, MouseWorldPos, MouseDir);

		FVector MouseToHighlight = PrimComp->GetComponentLocation() - MouseWorldPos;

		//Calculate desired mouse location using trig
		float CosAngle = FVector::DotProduct(MouseToHighlight.GetSafeNormal(), MouseDir);
		float DistanceAlongDir = MouseToHighlight.Size() * CosAngle;
		FVector NewLoc = MouseWorldPos + MouseDir * DistanceAlongDir;

		//Represent new location as an offset vector
		FVector Offset = NewLoc - PrimComp->GetComponentLocation();

		if (CurrentMouseDragType == EMouseDragType::HighlightXY && CurrentCursorMovable->IsMoveTypeAllowed(ECursorMoveType::XY))
		{
			//Get the scalar projection onto the right vector of the camera
			float RightTranslationLength = FVector::DotProduct(Offset, GetActorRightVector());
			FVector RightTranslation = GetActorRightVector() * RightTranslationLength;
			
			//Get the scalar projection onto the up vector of the camera but apply it to the forward vector using camera right and world up
			FVector ForwardDir = FVector::CrossProduct(GetActorRightVector(), FVector::UpVector);
			float ForwardTranslationLength = FVector::DotProduct(Offset, GetActorUpVector());
			FVector ForwardTranslation = ForwardDir * ForwardTranslationLength;

			PrimComp->AddWorldOffset(RightTranslation);
			PrimComp->AddWorldOffset(ForwardTranslation);
		}
		else if (CurrentMouseDragType == EMouseDragType::HighlightZ && CurrentCursorMovable->IsMoveTypeAllowed(ECursorMoveType::Z))
		{
			//Get the scalar projection onto the world up vector
			float UpTranslationLength = FVector::DotProduct(Offset, FVector::UpVector);
			FVector UpTranslation = FVector::UpVector * UpTranslationLength;

			PrimComp->AddWorldOffset(UpTranslation);
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

