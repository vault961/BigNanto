// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterSpawner.h"
#include "Components/SphereComponent.h"

// Sets default values
ACharacterSpawner::ACharacterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	Spawner = CreateDefaultSubobject<USphereComponent>(TEXT("Spawner"));
	Spawner->AttachTo(RootComponent);
}


