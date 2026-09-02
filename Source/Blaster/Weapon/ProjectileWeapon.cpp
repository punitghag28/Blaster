// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (!HasAuthority())
    {
        return;
    }

    APawn* InstiagatorPawn = Cast<APawn>(GetOwner());
    if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        // From Muzzle Flash Socket to HitLocation from TraceUnderCrosshairs
        FVector ToTarget = HitTarget - SocketTransform.GetLocation();
        FRotator TargetRotation = ToTarget.Rotation();
        if (ProjectileClass && InstiagatorPawn)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = InstiagatorPawn;

            UWorld* World = GetWorld();
            if (World)
            {
                World->SpawnActor<AProjectile>(ProjectileClass,
                    SocketTransform.GetLocation(),
                    TargetRotation,
                    SpawnParams);
            }
        }

    }
}
