// Fill out your copyright notice in the Description page of Project Settings.

#include "BallAimingFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "../TrajectoryDataProvider.h"
#include "../TennisNetActor.h"
#include <Kismet/KismetSystemLibrary.h>

#include "DrawDebugHelpers.h"

void FBallTrajectoryData::AddTrajectoryPoint(FVector PointLocation)
{
	FBallTrajectoryPoint TrajectoryPoint = FBallTrajectoryPoint(PointLocation);
	TrajectoryPoints.Add(TrajectoryPoint);
}

void FBallTrajectoryData::AddTrajectoryPoint(FVector PointLocation, FVector PointArriveTangent, FVector PointLeaveTangent)
{
	FBallTrajectoryPoint TrajectoryPoint = FBallTrajectoryPoint(PointLocation, PointArriveTangent, PointLeaveTangent);
	TrajectoryPoints.Add(TrajectoryPoint);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FName TrajectoryRowName, FVector StartLocation, FVector EndLocation, AActor* WorldContextActor)
{
	return GenerateTrajectoryData(RetrieveTrajectoryParamsFromDataProvider(TrajectoryRowName), StartLocation, EndLocation, WorldContextActor);
}

FBallTrajectoryData UBallAimingFunctionLibrary::GenerateTrajectoryData(FTrajectoryParams TrajParams, FVector StartLocation, FVector EndLocation, AActor* WorldContextActor /*= nullptr*/)
{
	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	FVector ShotDirectPath = EndLocation - StartLocation;
	ShotDirectPath.Z = 0;

	FVector TrajectoryDirection = ShotDirectPath.GetSafeNormal();

	float ShotDirectLength = ShotDirectPath.Size();

	if (TrajParams.TrajectoryCurve)
	{
		FRichCurve CurveData = TrajParams.TrajectoryCurve->FloatCurve;
		float CurveHeightConstant = StartLocation.Z / CurveData.Eval(0.f);
		float HeightConstantToUse = CurveHeightConstant;

		static const float MAX_HEIGHT = 200.f; //TODO(achester): Maybe there's a way to dynamically get this by checking the height of the character strike zone?
		static const float BALL_RADIUS = 10.f; //TODO(achester): Maybe there's a better way to get this value that is directly linked to the ball actor?

		static const float ExpectedCurveDuration = 1.f;
		static const int NumSegments = 20;

		TrajectoryData.BounceLocationIndex = NumSegments;

		static const float NetXPosition = 0.f; //TODO(achester): even though this likely will always be 0, it would be better to query this from the net actor to be safe
		static const float NetHeight = 112.f; //TODO(achester): find a way to populate this with some query so that if the net changes it will automatically update
		
		//If the trajectory would send the ball into the net, we try to adjust it upwards so that it clears the net (Within reason)
		bool bNeedsUpwardsAdjustment = false;
		float AdjustmentHeight = NetHeight + TrajParams.MinNetClearance + BALL_RADIUS;

		//Determine if upwards adjustment should be applied.  Due to validation, we check even for trajectories that aren't able to be adjusted upwards
		{
			//NOTE(achester): Alpha value for when the shot crosses the net is found by X = StartingPoint.X + A * DirectionVector.X, solved for A: A = X + -StartingPoint.X / DirectionVector.X
			float CrossNetAlphaValue = FMath::Abs(NetXPosition + -StartLocation.X / TrajectoryDirection.X) / ShotDirectLength;
			if (CurveData.Eval(CrossNetAlphaValue) * CurveHeightConstant < AdjustmentHeight)
			{
				bNeedsUpwardsAdjustment = TrajParams.bCanBeAdjustedUpwards;
				TrajectoryData.bShouldBeValidated = true;

				//If we cannot do an actual adjustment, come up with an index that can be used for validation
				if (!TrajParams.bCanBeAdjustedUpwards)
				{
					TrajectoryData.ValidateFromIndex = static_cast<int>(CrossNetAlphaValue * NumSegments) - 1; //Subtract 1 to make sure we're testing from before the net crossing
				}
			}
		}

		//Generate initial curve data by evaluating FloatCurve asset using 3d height and distance values
		for (int i = 0; i <= NumSegments; i++)
		{
			float CurveAlpha = static_cast<float>(i) / NumSegments;

			float CurveVal = CurveData.Eval(CurveAlpha * ExpectedCurveDuration);
			float CalculatedHeightVal = CurveVal * HeightConstantToUse;

			FVector TrajectoryPoint = StartLocation + TrajectoryDirection * CurveAlpha * ShotDirectLength;

			//The CurveHeightConstant represents the highest point on the curve.  If greater than our max height we need to adjust the curve down so that the highest point is at or below our max
			if (TrajParams.bCanBeAdjustedDownwards && !bNeedsUpwardsAdjustment && CurveHeightConstant > MAX_HEIGHT && i <= TrajParams.MaxHeightConformingIndex)
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
			if (TrajParams.bCanBeAdjustedDownwards && !bNeedsUpwardsAdjustment)
			{
				TrajectoryPoint.Z = FMath::Min(TrajectoryPoint.Z, MAX_HEIGHT);
			}

			TrajectoryData.AddTrajectoryPoint(TrajectoryPoint);
		}

		if (bNeedsUpwardsAdjustment && TrajParams.bCanBeAdjustedUpwards)
		{
			int AdjustmentPointIndex = -1;

			//NOTE(achester): This detection might be insufficient once shots that aren't straight lines are introduced (curving shots) 
			for (int i = 0; i < NumSegments; i++)
			{
				static const float SegmentLength = (static_cast<float>(1) / NumSegments) * ShotDirectLength;

				if (i == TrajParams.MaxAdjustmentIndex)
				{
					AdjustmentPointIndex = TrajParams.MaxAdjustmentIndex;

					break;
				}
				else if (FMath::Abs(TrajectoryData.TrajectoryPoints[i].Location.X - NetXPosition) <= SegmentLength)
				{
					AdjustmentPointIndex = (i > TrajParams.MinAdjustmentIndex) ? i : TrajParams.MinAdjustmentIndex;

					break;
				}
			}

			if (AdjustmentPointIndex >= 0)
			{
				float AdjustmentProportion = AdjustmentHeight / TrajectoryData.TrajectoryPoints[AdjustmentPointIndex].Location.Z;
				TrajectoryData.TrajectoryPoints[AdjustmentPointIndex].Location.Z = AdjustmentHeight;

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
						InterpolatedProportion = FMath::InterpEaseOut(1.f, AdjustmentProportion, static_cast<float>(NumSegments - i) / (NumSegments - AdjustmentPointIndex), 3.f);

						TrajectoryData.TrajectoryPoints[i].Location.Z = InterpolatedProportion * TrajectoryData.TrajectoryPoints[i].Location.Z;
					}
				}
			}

			TrajectoryData.bShouldBeValidated = true;
			TrajectoryData.ValidateFromIndex = AdjustmentPointIndex;
		}

		//Generate bounce trajectory data
		if (TrajParams.BounceTrajectoryCurve)
		{
			FRichCurve BounceCurveData = TrajParams.BounceTrajectoryCurve->FloatCurve;
			TrajectoryData.TrajectoryPoints[NumSegments].bSetTangent = true;

			const int NumBounceSegments = 10;
			const int TotalNumSegments = NumSegments + NumBounceSegments;

			//Skip NumSegments because that point should just be a repeat of the bounce location
			for (int i = NumSegments + 1; i <= TotalNumSegments; i++)
			{
				float CurveAlpha = static_cast<float>(i - NumSegments) / NumBounceSegments;

				float CurveVal = BounceCurveData.Eval(CurveAlpha * ExpectedCurveDuration);

				FVector TrajectoryPoint = EndLocation + TrajectoryDirection * CurveAlpha * ShotDirectLength * TrajParams.BounceLengthProportion;
				TrajectoryPoint.Z = CurveVal * TrajParams.BaseBounceHeight + BALL_RADIUS;

				TrajectoryData.AddTrajectoryPoint(TrajectoryPoint);
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

		if (TrajectoryData.TrajectoryPoints[i].bSetTangent)
		{
			//NOTE(achester): Despite not actually setting the values for ArriveTangent and LeaveTangent I think this ends up working by setting the tangents to length 0 so that there's effectively no manually set tangent and no auto tangent
			SplineComp->SetTangentsAtSplinePoint(i, (i > 0) ? TrajectoryData.TrajectoryPoints[i].ArriveTangent : FVector::ZeroVector, TrajectoryData.TrajectoryPoints[i].LeaveTangent, ESplineCoordinateSpace::World, false);
		}
	}

	SplineComp->UpdateSpline();
}

bool UBallAimingFunctionLibrary::ValidateTrajectorySplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp)
{
	//Trust that if we weren't adjusted upwards, the trajectory will clear the net
	if (!TrajectoryData.bShouldBeValidated)
	{
		return true;
	}
	
	static const float BALL_RADIUS = 10.f;

	//Starting from the adjustment index and ending at the bounce location, do a series of collision tests to determine if the ball will clear the net
	FVector AdjustmentLocation = SplineComp->GetWorldLocationAtSplinePoint(TrajectoryData.ValidateFromIndex);
	FVector BounceLocation = SplineComp->GetWorldLocationAtSplinePoint(TrajectoryData.BounceLocationIndex);
	FVector TestDirection = BounceLocation - AdjustmentLocation;

	const int NumTests = 20;
	float TestInterval = TestDirection.Size() / NumTests;

	TestDirection.Normalize();

	for (int i = 1; i <= NumTests; i++)
	{
		FVector TestLocation = SplineComp->FindLocationClosestToWorldLocation(AdjustmentLocation + TestDirection * i * TestInterval, ESplineCoordinateSpace::World);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel5)); //Object Channel TennisNet

		TArray<AActor*> OutActors;

		UKismetSystemLibrary::SphereOverlapActors(SplineComp, TestLocation, BALL_RADIUS, ObjectTypes, ATennisNetActor::StaticClass(), TArray<AActor*>(), OutActors);
		
		if (OutActors.Num() > 0)
		{
			//DrawDebugSphere(SplineComp->GetWorld(), TestLocation, 10.f, 10, FColor::Red, false, 1.f, 0, 1.f);

			return false;
		}
		/*else
		{
			DrawDebugSphere(SplineComp->GetWorld(), TestLocation, 10.f, 10, FColor::Yellow, false, 1.f, 0, 1.f);
		}*/
	}

	return true;
}

FTrajectoryParams UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(FName TrajectoryRowName)
{
	const UDataTable* TrajParamsDT = UTrajectoryDataProvider::GetDefaultTrajectoryParamsDT();

	ensureMsgf(TrajParamsDT, TEXT("UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider - Unable to get valid reference to Trajectory Params DT"));

	FString ContextStr;
	FTrajectoryParams* TrajParamsPtr = TrajParamsDT->FindRow<FTrajectoryParams>(TrajectoryRowName, ContextStr);

	ensureMsgf(TrajParamsPtr, TEXT("UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider - Unable to retrieve Trajectory Params from DT"));

	return *TrajParamsPtr;
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
