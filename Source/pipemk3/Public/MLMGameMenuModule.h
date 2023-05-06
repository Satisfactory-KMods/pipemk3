// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGFactorySettings.h"
#include "Module/GameWorldModule.h"
#include "UObject/Object.h"
#include "Module/MenuWorldModule.h"
#include "MLMGameMenuModule.generated.h"

/**
 * 
 */
UCLASS()
class PIPEMK3_API UMLMGameMenuModule : public UMenuWorldModule
{
	GENERATED_BODY()

	UMLMGameMenuModule();
	virtual void DispatchLifecycleEvent( ELifecyclePhase Phase ) override;
	void InitHooks();
	void SetupSettings();

	TSubclassOf< UFGFactorySettings > mFactorySettings;
	TSubclassOf< UFGFactorySettings > mModdedFactorySettings;
};
