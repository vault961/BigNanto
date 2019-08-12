// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoCameraShake.h"

UBigNantoCameraShake::UBigNantoCameraShake()
{
	OscillationDuration = 0.3f;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.05f;

	LocOscillation.Y.Amplitude = FMath::RandRange(10.0f, 20.0f);
	LocOscillation.Y.Frequency = FMath::RandRange(40.0f, 50.0f);

	LocOscillation.Z.Amplitude = FMath::RandRange(10.0f, 20.0f);
	LocOscillation.Z.Frequency = FMath::RandRange(40.0f, 50.0f);
}