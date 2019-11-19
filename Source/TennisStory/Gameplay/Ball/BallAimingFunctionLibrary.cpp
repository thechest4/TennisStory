// Fill out your copyright notice in the Description page of Project Settings.

#include "BallAimingFunctionLibrary.h"
#include "Components/SplineComponent.h"

void FBallTrajectoryData::AddTrajectoryPoint(FVector PointLocation, FVector PointTangent)
{
	FBallTrajectoryPoint TrajectoryPoint = FBallTrajectoryPoint(PointLocation, PointTangent);
	TrajectoryPoints.Add(TrajectoryPoint);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(UCurveFloat* TrajectoryCurve, FVector StartLocation, FVector EndLocation, float ApexHeight, float TangentLength)
{
	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	FVector MidPoint = (EndLocation - StartLocation) / 2.f + StartLocation;
	
	FVector DirectionVec = EndLocation - StartLocation;
	DirectionVec.Z = 0.f;
	DirectionVec.Normalize();

	float LeaveSlope = 1.f;
	float ArriveSlope = 1.f;
	if (TrajectoryCurve)
	{
		FRichCurve CurveData = TrajectoryCurve->FloatCurve;
		LeaveSlope = CurveData.GetFirstKey().LeaveTangent;
		ArriveSlope = CurveData.GetLastKey().ArriveTangent;
	}

	float LeaveAngle = FMath::RadiansToDegrees(FMath::Atan(LeaveSlope));
	float ArriveAngle = FMath::RadiansToDegrees(FMath::Atan(ArriveSlope));

	FVector RightVec = FVector::CrossProduct(FVector::UpVector, DirectionVec);
	FVector StartTangent = DirectionVec.RotateAngleAxis(-LeaveAngle, RightVec);
	FVector EndTangent = DirectionVec.RotateAngleAxis(-ArriveAngle, RightVec);

	TrajectoryData.AddTrajectoryPoint(StartLocation, StartTangent * TangentLength);

	MidPoint += FVector(0.f, 0.f, ApexHeight);

	TrajectoryData.AddTrajectoryPoint(MidPoint, DirectionVec * TangentLength);
	
	TrajectoryData.AddTrajectoryPoint(EndLocation, EndTangent * TangentLength);

	return TrajectoryData;
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryDataFromSplineComp(USplineComponent* SplineComp)
{
	if (!SplineComp)
	{
		return FBallTrajectoryData();
	}

	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	for (int i = 0; i < SplineComp->GetNumberOfSplinePoints(); i++)
	{
		FVector Location = SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector Tangent = SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		TrajectoryData.AddTrajectoryPoint(Location, Tangent);
	}

	return TrajectoryData;
}

void UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp)
{
	if (!SplineComp)
	{
		return;
	}

	SplineComp->ClearSplinePoints();

	for (int i = 0; i < TrajectoryData.TrajectoryPoints.Num(); i++)
	{
		SplineComp->AddSplinePointAtIndex(TrajectoryData.TrajectoryPoints[i].Location, i, ESplineCoordinateSpace::World, false);
		SplineComp->SetTangentAtSplinePoint(i, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);
	}

	SplineComp->UpdateSpline();
}

void UBallAimingFunctionLibrary::DebugVisualizeSplineComp(USplineComponent* SplineComp)
{
	/*#include "DrawDebugHelpers.h"
	for (float i = 0.f; i < SplineComp->Duration; i += SplineComp->Duration / 15.f)
	{
		FVector SplineLoc = SplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

		DrawDebugSphere(SplineComp->GetOwner()->GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 3.f);
	}*/
}
