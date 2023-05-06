// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGFactorySettings.h"
#include "Configuration/ModConfiguration.h"
#include "Module/GameWorldModule.h"
#include "Module/MenuWorldModule.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"
#include "MLMGameWorldModule.generated.h"

/**
 * 
 */
UCLASS()
class PIPEMK3_API UMLMGameWorldModule : public UGameWorldModule
{
	GENERATED_BODY()

	// native helper
	FORCEINLINE static UConfigPropertySection* GetPropertySection( TSubclassOf< UModConfiguration > Config )
	{
		return Cast< UConfigPropertySection >( URuntimeBlueprintFunctionLibrary::GetModConfigurationPropertyByClass( Config ) );
	}

	template< class T >
	FORCEINLINE static T* GetPropertyByKey( TSubclassOf< UModConfiguration > Config, FString Key )
	{
		if( UConfigPropertySection* Configuration = GetPropertySection( Config ) )
		{
			return Cast< T >( URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertySectionToConfigProperty( Configuration, Key ) );
		}
		return nullptr;
	}

	FORCEINLINE static UConfigProperty* GetConfigPropertyByKey( TSubclassOf< UModConfiguration > Config, FString Key )
	{
		if( UConfigPropertySection* Configuration = GetPropertySection( Config ) )
		{
			return URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertySectionToConfigProperty( Configuration, Key );
		}
		return nullptr;
	}

	FORCEINLINE static void SaveProperty( TSubclassOf< UModConfiguration > Config, FString Key )
	{
		if( UConfigPropertySection* Section = GetPropertySection( Config ) )
		{
			Section->MarkDirty();
			if( UConfigProperty* Property = GetPropertyByKey< UConfigProperty >( Config, Key ) )
			{
				Property->MarkDirty();
			}
		}
	}


	UMLMGameWorldModule();
	void ApplyConfigSettings();
	virtual void DispatchLifecycleEvent( ELifecyclePhase Phase ) override;
	void InitHooks();
	void SetupSettings();

	TSubclassOf< UFGFactorySettings > mFactorySettings;
	TSubclassOf< UFGFactorySettings > mModdedFactorySettings;
	TSubclassOf< UModConfiguration > mConfig;

	float mMaxContentMultiplierMk1;
	float mMaxContentMultiplierMk2;
	float mMaxContentMultiplierMk3;
	float mFlowLimit;
};
