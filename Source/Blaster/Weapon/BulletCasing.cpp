#include "Weapon/BulletCasing.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
}

