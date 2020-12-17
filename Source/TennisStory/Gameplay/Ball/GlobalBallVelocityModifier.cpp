// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalBallVelocityModifier.h"

void UGlobalBallVelocityModifier::CalculateGlobalVelocityModifiers(FBallTrajectoryData TrajectoryData, TArray<float>& MultiplicativeModifiers)
{
	MultiplicativeModifiers.Add(CalculateDistanceModifier(TrajectoryData));
	MultiplicativeModifiers.Add(CalculateAdjustmentModifier(TrajectoryData));
}

float UGlobalBallVelocityModifier::CalculateDistanceModifier(FBallTrajectoryData TrajectoryData)
{
	FDistanceModifierRules Rules = TrajectoryData.DistanceModifierRules;

	if (Rules.bEnableDistanceModifier && Rules.ModifierCurve)
	{
		float ActualDistanceProportion = TrajectoryData.ShotDistance / Rules.ReferenceDistance;

		return Rules.ModifierCurve->FloatCurve.Eval(ActualDistanceProportion);
	}

	return 1.f;
}

float UGlobalBallVelocityModifier::CalculateAdjustmentModifier(FBallTrajectoryData TrajectoryData)
{
	FHeightAdjustmentModifierRules Rules = TrajectoryData.AdjustmentModifierRules;

	if (Rules.bEnableAdjustmentModifier && Rules.ModifierCurve)
	{
		//if adjustment didn't occur, return the max value on the curve (which should occur at X = 1.f)
		//This is to enable curves to return a multiplier higher than 1, without penalizing a shot for not being adjusted upwards
		if (!TrajectoryData.bWasAdjustedUpwards)
		{
			return Rules.ModifierCurve->FloatCurve.Eval(1.f);
		}

		return Rules.ModifierCurve->FloatCurve.Eval(TrajectoryData.AdjustmentProportion);
	}

	return 1.f;
}
