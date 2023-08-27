// Copyright Epic Games, Inc. All Rights Reserved.

#include "VertexColorFromHitResultBPLibrary.h"
#include "VertexColorFromHitResult.h"
#include "Kismet/KismetMathLibrary.h"

UVertexColorFromHitResultBPLibrary::UVertexColorFromHitResultBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UVertexColorFromHitResultBPLibrary::VertexColorFromHitResultSampleFunction(float Param)
{
	return -1;
}

FVector ConvertFvector3fToFVector(FVector3f Vector3F)
{
	return  FVector(Vector3F.X,Vector3F.Y,Vector3F.Z);
}
bool UVertexColorFromHitResultBPLibrary::VertexColorFromHitResult(UPARAM(ref)FHitResult& HitResult, FLinearColor& OutColor)
{
	UPrimitiveComponent* PrimitiveComponent = HitResult.Component.Get();
	if (PrimitiveComponent)
	{
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(PrimitiveComponent);
		if (StaticMeshComponent)
		{
			UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if(Mesh && Mesh->bAllowCPUAccess)
			{
				FStaticMeshLODResources& LODResources = Mesh->GetRenderData()->LODResources[0];
				TArray<FColor> VertexColors;
				if (StaticMeshComponent->LODData.Num()>0 && StaticMeshComponent->LODData[0].OverrideVertexColors)
				{
					StaticMeshComponent->LODData[0].OverrideVertexColors->GetVertexColors(VertexColors);
				}
				else
				{
					if (LODResources.bHasColorVertexData)
					{
						LODResources.VertexBuffers.ColorVertexBuffer.GetVertexColors(VertexColors);
					}
				}
				VertexColors.SetNumZeroed(LODResources.GetNumVertices());
				
					int FaceIndex = HitResult.FaceIndex;
					
					int32 Index0 = LODResources.IndexBuffer.GetIndex(FaceIndex * 3 + 0);
					int32 Index1 = LODResources.IndexBuffer.GetIndex(FaceIndex * 3 + 1);
					int32 Index2 = LODResources.IndexBuffer.GetIndex(FaceIndex * 3 + 2);
					
					FLinearColor Color0 = VertexColors[Index0];
					FLinearColor Color1 = VertexColors[Index1];
					FLinearColor Color2 = VertexColors[Index2];

					FVector Position0 =  ConvertFvector3fToFVector(LODResources.VertexBuffers.PositionVertexBuffer.VertexPosition(Index0));
					FVector Position1 =  ConvertFvector3fToFVector(LODResources.VertexBuffers.PositionVertexBuffer.VertexPosition(Index1));
					FVector Position2 =  ConvertFvector3fToFVector(LODResources.VertexBuffers.PositionVertexBuffer.VertexPosition(Index2));

					FVector LocalPosition = PrimitiveComponent->GetComponentToWorld().InverseTransformPosition(HitResult.Location);

					FVector BaryCoords = FMath::ComputeBaryCentric2D(LocalPosition, Position0, Position1, Position2);

					OutColor = (Color0*BaryCoords.X + Color1*BaryCoords.Y + Color2*BaryCoords.Z);
					return true;
			}
			UE_LOG(LogTemp,Warning,TEXT("bAllowCPUAccess = false"));
		}
		UE_LOG(LogTemp,Warning,TEXT("Only StaticMeshComponent Allowed"));
	}
	return false;
}

