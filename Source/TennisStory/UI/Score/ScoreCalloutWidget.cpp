// Fill out your copyright notice in the Description page of Project Settings.

#include "ScoreCalloutWidget.h"


void UScoreCalloutWidget::BroadcastWidgetFinishedEvent()
{
	OnCalloutWidgetFinished().Broadcast();
}
