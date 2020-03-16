// Fill out your copyright notice in the Description page of Project Settings.

#include "ServiceCalloutWidget.h"

void UServiceCalloutWidget::BroadcastWidgetFinishedEvent()
{
	OnServiceCalloutWidgetFinished().Broadcast();
}