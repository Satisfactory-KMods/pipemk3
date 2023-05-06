// Copyright Coffee Stain Studios. All Rights Reserved.


#include "MLMGameMenuModule.h"

#include "FGPipeConnectionComponent.h"
#include "FGPipeConnectionFactory.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Patching/NativeHookManager.h"

UMLMGameMenuModule::UMLMGameMenuModule()
{
	bRootModule = true;
	mFactorySettings = LoadClass< UFGFactorySettings >( NULL, TEXT( "/Game/FactoryGame/Buildable/Factory/BP_FactorySettings.BP_FactorySettings_C" ) );
	mModdedFactorySettings = LoadClass< UFGFactorySettings >( NULL, TEXT( "/pipemk3/MLM_FactorySettings.MLM_FactorySettings_C" ) );
}

void UMLMGameMenuModule::DispatchLifecycleEvent( ELifecyclePhase Phase )
{
	Super::DispatchLifecycleEvent( Phase );

	if( Phase == ELifecyclePhase::POST_INITIALIZATION )
	{
		SetupSettings();
		InitHooks();
	}
}

void UMLMGameMenuModule::InitHooks()
{
}

void UMLMGameMenuModule::SetupSettings()
{
	fgcheck( mFactorySettings );
	fgcheck( mModdedFactorySettings );
	UFGFactorySettings* Settings = mFactorySettings.GetDefaultObject();
	UFGFactorySettings* ModdedSettings = mModdedFactorySettings.GetDefaultObject();

	Settings->mFluidToInventoryStackRate = ModdedSettings->mFluidToInventoryStackRate;
	Settings->mInventoryStackToFluidRate = ModdedSettings->mInventoryStackToFluidRate;
	Settings->mAddedPipeProductionPressure = ModdedSettings->mAddedPipeProductionPressure;
	Settings->mViscosityToPuddlePairs = ModdedSettings->mViscosityToPuddlePairs;
}
