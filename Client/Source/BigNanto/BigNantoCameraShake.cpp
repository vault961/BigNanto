// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoCameraShake.h"

UBigNantoCameraShake::UBigNantoCameraShake()
{
	OscillationDuration = 0.5f;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.05f;

	LocOscillation.Y.Amplitude = FMath::RandRange(4.0f, 5.0f);
	LocOscillation.Y.Frequency = FMath::RandRange(30.0f, 40.0f);

	LocOscillation.Z.Amplitude = FMath::RandRange(4.0f, 5.0f);
	LocOscillation.Z.Frequency = FMath::RandRange(30.0f, 40.0f);
}