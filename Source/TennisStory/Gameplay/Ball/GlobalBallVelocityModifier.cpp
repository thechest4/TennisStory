// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalBallVelocityModifier.h"

void UGlobalBallVelocityModifier::CalculateGlobalVelocityModifiers(FBallTrajectoryData TrajectoryData, TArray<float>& MultiplicativeModifiers)
{
	MultiplicativeModifiers.Add(CalculateDistanceModifier(TrajectoryData));
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
