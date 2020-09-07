// Fill out your copyright notice in the Description page of Project Settings.

#include "BallAimingFunctionLibrary.h"
#include "Components/SplineComponent.h"

void FBallTrajectoryData::AddTrajectoryPoint(FVector PointLocation, FVector PointTangent)
{
	FBallTrajectoryPoint TrajectoryPoint = FBallTrajectoryPoint(PointLocation, PointTangent);
	TrajectoryPoints.Add(TrajectoryPoint);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData_Old(UCurveFloat* TrajectoryCurve, FVector StartLocation, FVector EndLocation, float ApexHeight, float TangentLength)
{
	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	FVector MidPoint = (EndLocation - StartLocation) / 2.f + StartLocation;
	
	FVector DirectPath = EndLocation - StartLocation;
	TrajectoryData.TrajectoryDistance = DirectPath.Size();

	FVector DirectionVec = DirectPath;
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

	TrajectoryData.ApexHeight = MidPoint.Z;
	TrajectoryData.AddTrajectoryPoint(MidPoint, DirectionVec * TangentLength);
	TrajectoryData.AddTrajectoryPoint(EndLocation, EndTangent * TangentLength);
	
	TrajectoryData.TrajectoryEndLocation = EndLocation;

	TrajectoryData.bSetTangents = true;

	return TrajectoryData;
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(UCurveFloat* TrajectoryCurve, FVector StartLocation, FVector EndLocation)
{
	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	FVector DirectPath = EndLocation - StartLocation;
	DirectPath.Z = 0;
	
	FVector TrajectoryDirection = DirectPath.GetSafeNormal();

	float DirectLength = DirectPath.Size();

	if (TrajectoryCurve)
	{
		FRichCurve CurveData = TrajectoryCurve->FloatCurve;
		
		const float ExpectedCurveDuration = 1.f;
		const int NumSegments = 10;
		for (int i = 0; i <= NumSegments; i++)
		{
			float CurveAlpha = static_cast<float>(i) / NumSegments;

			float CurveVal = CurveData.Eval(CurveAlpha * ExpectedCurveDuration);

			FVector TrajectoryPoint = StartLocation + TrajectoryDirection * CurveAlpha * DirectLength;
			TrajectoryPoint.Z = CurveVal * StartLocation.Z;

			TrajectoryData.AddTrajectoryPoint(TrajectoryPoint, FVector::ZeroVector);
		}
	}

	TrajectoryData.bSetTangents = false;

	return TrajectoryData;
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FTrajectoryParams_Old TrajParams_Old, FVector StartLocation, FVector EndLocation)
{
	return GenerateTrajectoryData_Old(TrajParams_Old.TrajectoryCurve, StartLocation, EndLocation, TrajParams_Old.ApexHeight, TrajParams_Old.TangentLength);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FTrajectoryParams_New TrajParams_New, FVector StartLocation, FVector EndLocation)
{
	return GenerateTrajectoryData(TrajParams_New.TrajectoryCurve, StartLocation, EndLocation);
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

		if (TrajectoryData.bSetTangents)
		{
			SplineComp->SetTangentAtSplinePoint(i, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);
		}
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
