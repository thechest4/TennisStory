// Fill out your copyright notice in the Description page of Project Settings.

#include "BallAimingFunctionLibrary.h"
#include "Components/SplineComponent.h"

#include "DrawDebugHelpers.h"

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

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FTrajectoryParams_Old TrajParams_Old, FVector StartLocation, FVector EndLocation)
{
	return GenerateTrajectoryData_Old(TrajParams_Old.TrajectoryCurve, StartLocation, EndLocation, TrajParams_Old.ApexHeight, TrajParams_Old.TangentLength);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FTrajectoryParams_New TrajParams, FVector StartLocation, FVector EndLocation, AActor* WorldContextActor)
{
	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	FVector DirectPath = EndLocation - StartLocation;
	DirectPath.Z = 0;

	FVector TrajectoryDirection = DirectPath.GetSafeNormal();

	float DirectLength = DirectPath.Size();

	if (TrajParams.TrajectoryCurve)
	{
		FRichCurve CurveData = TrajParams.TrajectoryCurve->FloatCurve;
		float CurveHeightConstant = StartLocation.Z / CurveData.Eval(0.f);
		float HeightConstantToUse = CurveHeightConstant;

		static const float MAX_HEIGHT = 200.f; //TODO(achester): Maybe there's a way to dynamically get this by checking the height of the character strike zone?
		static const float BALL_RADIUS = 10.f; //TODO(achester): Maybe there's a better way to get this value that is directly linked to the ball actor?

		static const float ExpectedCurveDuration = 1.f;
		static const int NumSegments = 20;

		static const float NetXPosition = 0.f; //TODO(achester): even though this likely will always be 0, it would be better to query this from the net actor to be safe
		static const float NetHeight = 112.f; //TODO(achester): find a way to populate this with some query so that if the net changes it will automatically update
		
		//If the trajectory would send the ball into the net, we try to adjust it upwards so that it clears the net (Within reason)
		bool bNeedsUpwardsAdjustment = false;
		float AdjustmentHeight = NetHeight + TrajParams.MinNetClearance + BALL_RADIUS;
		
		//NOTE(achester): Alpha value for when the shot crosses the net is found by X = StartingPoint.X + A * DirectionVector.X, solved for A: A = X + -StartingPoint.X / DirectionVector.X
		float CrossNetAlphaValue = FMath::Abs(NetXPosition + -StartLocation.X / TrajectoryDirection.X) / DirectLength;
		if (CurveData.Eval(CrossNetAlphaValue) * CurveHeightConstant < AdjustmentHeight)
		{
			bNeedsUpwardsAdjustment = true;
		}

		//Generate initial curve data by evaluating FloatCurve asset using 3d height and distance values
		for (int i = 0; i <= NumSegments; i++)
		{
			float CurveAlpha = static_cast<float>(i) / NumSegments;

			float CurveVal = CurveData.Eval(CurveAlpha * ExpectedCurveDuration);
			float CalculatedHeightVal = CurveVal * HeightConstantToUse;

			FVector TrajectoryPoint = StartLocation + TrajectoryDirection * CurveAlpha * DirectLength;

			//The CurveHeightConstant represents the highest point on the curve.  If greater than our max height we need to adjust the curve down so that the highest point is at or below our max
			if (!bNeedsUpwardsAdjustment && CurveHeightConstant > MAX_HEIGHT && i <= TrajParams.MaxHeightConformingIndex)
			{
				HeightConstantToUse = FMath::Lerp(MAX_HEIGHT, CurveHeightConstant, (TrajParams.MaxHeightConformingIndex - i) / static_cast<float>(TrajParams.MaxHeightConformingIndex));

				//If we're still adjusting the curve downwards (i <= MaxHeightConformingIndex) we can run into scenarios where the calculated height is lower than our starting height.  If so, just raise the height to form a straight line
				if (CalculatedHeightVal < StartLocation.Z)
				{
					CalculatedHeightVal = FMath::Min(MAX_HEIGHT, StartLocation.Z);
				}
			}

			TrajectoryPoint.Z = CalculatedHeightVal + (static_cast<float>(i) / NumSegments) * BALL_RADIUS;

			//Make sure the final height does not exceed MAX_HEIGHT if we're adjusting downwards
			if (!bNeedsUpwardsAdjustment)
			{
				TrajectoryPoint.Z = FMath::Min(TrajectoryPoint.Z, MAX_HEIGHT);
			}

			TrajectoryData.AddTrajectoryPoint(TrajectoryPoint, FVector::ZeroVector);
		}

		if (bNeedsUpwardsAdjustment)
		{
			int AdjustmentPointIndex = -1;

			//NOTE(achester): This detection might be insufficient once shots that aren't straight lines are introduced (curving shots) 
			for (int i = 0; i < NumSegments; i++)
			{
				static const float SegmentLength = (static_cast<float>(1) / NumSegments) * DirectLength;

				if (i == TrajParams.MaxAdjustmentIndex)
				{
					AdjustmentPointIndex = TrajParams.MaxAdjustmentIndex;

					GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Blue, FString::Printf(TEXT("Using Max Adjustment Index: %d"), AdjustmentPointIndex));

					break;
				}
				else if (FMath::Abs(TrajectoryData.TrajectoryPoints[i].Location.X - NetXPosition) <= SegmentLength)
				{
					AdjustmentPointIndex = (i > TrajParams.MinAdjustmentIndex) ? i : TrajParams.MinAdjustmentIndex;
					GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Blue, FString::Printf(TEXT("Adjustment Index: %d"), AdjustmentPointIndex));

					break;
				}
			}

			if (AdjustmentPointIndex >= 0)
			{
				float AdjustmentProportion = AdjustmentHeight / TrajectoryData.TrajectoryPoints[AdjustmentPointIndex].Location.Z;
				TrajectoryData.TrajectoryPoints[AdjustmentPointIndex].Location.Z = AdjustmentHeight;

				UE_LOG(LogTemp, Warning, TEXT("StartingAdjustment: %f"), AdjustmentProportion);

				float InterpolatedProportion = 1.f;
				//Iterate through all segments and apply adjustment
				for (int i = 0; i <= NumSegments; i++)
				{
					if (i < AdjustmentPointIndex)
					{
						InterpolatedProportion = FMath::InterpEaseOut(1.f, AdjustmentProportion, static_cast<float>(i) / AdjustmentPointIndex, 2.f);
						TrajectoryData.TrajectoryPoints[i].Location.Z = InterpolatedProportion * TrajectoryData.TrajectoryPoints[i].Location.Z;
					}
					else if (i > AdjustmentPointIndex)
					{
						//TrajectoryData.TrajectoryPoints[i].Location.Z = AdjustmentProportion * TrajectoryData.TrajectoryPoints[i].Location.Z;

						//InterpolatedProportion = FMath::InterpEaseOut(AdjustmentProportion, 1.f, static_cast<float>(i - AdjustmentPointIndex) / (TrajectoryData.TrajectoryPoints.Num() - AdjustmentPointIndex), 2.f);
						InterpolatedProportion = FMath::InterpEaseOut(1.f, AdjustmentProportion, static_cast<float>(NumSegments - i) / (NumSegments - AdjustmentPointIndex), 3.f);

						TrajectoryData.TrajectoryPoints[i].Location.Z = InterpolatedProportion * TrajectoryData.TrajectoryPoints[i].Location.Z;
					}
				}
			}
		}
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

		if (TrajectoryData.bSetTangents)
		{
			SplineComp->SetTangentAtSplinePoint(i, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);

			//SplineComp->SetTangentsAtSplinePoint(i, (i > 0) ? TrajectoryData.TrajectoryPoints[i - 1].Tangent : FVector::ZeroVector, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);
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
