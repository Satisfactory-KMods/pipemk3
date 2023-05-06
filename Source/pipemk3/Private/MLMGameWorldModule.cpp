// Copyright Coffee Stain Studios. All Rights Reserved.
#include "MLMGameWorldModule.h"

#include "FGFactorySettings.h"
#include "FGSchematic.h"

#include "FGPipeConnectionFactory.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "Patching/NativeHookManager.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"

UMLMGameWorldModule::UMLMGameWorldModule(): mMaxContentMultiplierMk1( 0 ), mMaxContentMultiplierMk2( 0 ), mMaxContentMultiplierMk3( 0 ), mFlowLimit( 0 )
{
	bRootModule = true;
	mFactorySettings = LoadClass< UFGFactorySettings >( NULL, TEXT( "/Game/FactoryGame/Buildable/Factory/BP_FactorySettings.BP_FactorySettings_C" ) );
	mModdedFactorySettings = LoadClass< UFGFactorySettings >( NULL, TEXT( "/pipemk3/MLM_FactorySettings.MLM_FactorySettings_C" ) );
	mConfig = LoadClass< UModConfiguration >( NULL, TEXT( "/pipemk3/Config_pipemk3.Config_pipemk3_C" ) );
}

void UMLMGameWorldModule::ApplyConfigSettings()
{
	fgcheck( mConfig );

	mMaxContentMultiplierMk1 = GetPropertyByKey< UConfigPropertyFloat >( mConfig, TEXT( "MaxContentMultiplierMk1" ) )->Value;
	mMaxContentMultiplierMk2 = GetPropertyByKey< UConfigPropertyFloat >( mConfig, TEXT( "MaxContentMultiplierMk2" ) )->Value;
	mMaxContentMultiplierMk3 = GetPropertyByKey< UConfigPropertyFloat >( mConfig, TEXT( "MaxContentMultiplierMk3" ) )->Value;
	mFlowLimit = GetPropertyByKey< UConfigPropertyFloat >( mConfig, TEXT( "FlowLimit" ) )->Value;
}

void UMLMGameWorldModule::DispatchLifecycleEvent( ELifecyclePhase Phase )
{
	Super::DispatchLifecycleEvent( Phase );

	if( Phase == ELifecyclePhase::CONSTRUCTION )
	{
		TSubclassOf< UFGSchematic > Schematic = LoadClass< UFGSchematic >( NULL, TEXT( "/pipemk3/Schematics/Schematic_PipelineMk3.Schematic_PipelineMk3_C" ) );
		mSchematics.AddUnique( Schematic );

		ApplyConfigSettings();
		SetupSettings();
		InitHooks();
	}
}

void UMLMGameWorldModule::InitHooks()
{
	AFGBuildablePipeline* PipelineDefault = GetMutableDefault< AFGBuildablePipeline >();
	UFGPipeConnectionFactory* ConnectionFactory = GetMutableDefault< UFGPipeConnectionFactory >();
	AFGBuildablePipelineAttachment* PipelineJunction = GetMutableDefault< AFGBuildablePipelineAttachment >();
	AFGBuildablePipeReservoir* BuildablePipeReservoir = GetMutableDefault< AFGBuildablePipeReservoir >();

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::GetUserFlowLimit, [](auto& Scope, const AFGBuildablePipelinePump* Pump) { if(ensure(Pump)) { float FlowLimit = Pump->mUserFlowLimit; if(Pump->GetDefaultFlowLimit() > 10 && FlowLimit >= 0.0f) { FlowLimit *= 2; } Scope.Override(FlowLimit); } } )

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::SetUserFlowLimit, [&](auto& Scope, AFGBuildablePipelinePump* Pump, float rate) { if(ensure(Pump)) { if(Pump->GetDefaultFlowLimit() > 10 && rate >= 0.0f) { rate /= 2; }

	                  //UE_LOG(LogTemp, Error, TEXT("FluidTEST: SetUserFlowLimit FlowRate: %f"), rate)

	                  Pump->mUserFlowLimit = rate; Pump->mFluidBox.FlowLimit = Pump->GetFlowLimit(); Scope.Cancel(); } } )

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::GetFlowLimit, [&](auto& Scope, const AFGBuildablePipelinePump* Pump) { if(ensure(Pump)) { float FlowLimit = Pump->mUserFlowLimit < 0.0f ? Pump->mDefaultFlowLimit : Pump->mUserFlowLimit; if(Pump->GetDefaultFlowLimit() > 10 && Pump->mUserFlowLimit >= 0.0f) { FlowLimit *= 2; } Scope.Override(FlowLimit); } } )

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::GetFlowLimitPct, [&](auto& Scope, const AFGBuildablePipelinePump* Pump) { if(ensure(Pump)) { Scope.Override(Pump->GetFlowLimit() / Pump->GetDefaultFlowLimit()); } } )

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::UpdateDefaultFlowLimit, [&](auto& Scope, AFGBuildablePipelinePump* Pump) { if(ensure(Pump)) { Scope(Pump); TArray< UFGPipeConnectionComponent* > Connections = Pump->mPipeConnections; float MaxFlow = 5.0f; for(UFGPipeConnectionComponent* ConnectionComponent: Connections) { if(ConnectionComponent->IsConnected()) { AFGBuildablePipeline* Pipe = Cast<AFGBuildablePipeline>(ConnectionComponent->GetConnection()->GetOwner()); if(Pipe) { MaxFlow = FMath::Max(MaxFlow, Pipe->mFlowLimit); } } } Pump->mDefaultFlowLimit = MaxFlow; Pump->UpdateFlowLimitOnFluidBox(); Scope.Cancel(); } } )

	SUBSCRIBE_METHOD( AFGBuildablePipelinePump::UpdateFlowLimitOnFluidBox, [&](auto& Scope, AFGBuildablePipelinePump* Pump) { if(ensure(Pump)) { Scope(Pump); Pump->mFluidBox.FlowLimit = Pump->GetFlowLimit(); Scope.Cancel(); } } )

	SUBSCRIBE_METHOD_VIRTUAL_AFTER( AFGBuildablePipeline::BeginPlay, PipelineDefault, [&](AFGBuildablePipeline* Pipeline) { if(ensureAlways(Pipeline)) { if(Pipeline->mFlowLimit < 9.0f) { Pipeline->mFluidBox.MaxContent = Pipeline->GetLength() * mMaxContentMultiplierMk1 / 100; } else if(Pipeline->mFlowLimit < 15.0f) { Pipeline->mFluidBox.MaxContent = Pipeline->GetLength() * mMaxContentMultiplierMk2 / 100; } else { Pipeline->mFluidBox.MaxContent = Pipeline->GetLength() * mMaxContentMultiplierMk3 / 100; } Pipeline->mFluidBox.FlowLimit = Pipeline->mFlowLimit; for (UFGPipeConnectionComponent* PipeConnection : Pipeline->GetPipeConnections()) { if(ensure(PipeConnection)) { if(PipeConnection->IsConnected()) { if(AFGBuildablePipelinePump* Pump = Cast<AFGBuildablePipelinePump>(PipeConnection->GetConnection()->GetOwner())) { Pump->mDefaultFlowLimit = Pipeline->mFlowLimit; Pump->UpdateDefaultFlowLimit(); Pump->UpdateFlowLimitOnFluidBox(); } } } } } } )

	SUBSCRIBE_METHOD_VIRTUAL_AFTER( UFGPipeConnectionFactory::BeginPlay, ConnectionFactory, [&](UFGPipeConnectionFactory* Connection) { if(ensureAlways(Connection)) { Connection->mFluidBox.MaxContent *= 2; } } )

	SUBSCRIBE_METHOD_VIRTUAL_AFTER( AFGBuildablePipeReservoir::BeginPlay, BuildablePipeReservoir, [&](AFGBuildablePipeReservoir* Reservoir) { if(ensureAlways(Reservoir)) { Reservoir->mFluidBox.FlowLimit = mFlowLimit; } } )

	SUBSCRIBE_METHOD_VIRTUAL_AFTER( AFGBuildablePipelineAttachment::BeginPlay, PipelineJunction, [&](AFGBuildablePipelineAttachment* PipelineAttachment) { if(ensureAlways(PipelineAttachment)) { PipelineAttachment->mFluidBox.FlowLimit = mFlowLimit; if(AFGBuildablePipelinePump* Pump = Cast<AFGBuildablePipelinePump>(PipelineAttachment)) { Pump->mDefaultFlowLimit = mFlowLimit; } } } )

	SUBSCRIBE_METHOD_VIRTUAL( UFGPipeConnectionFactory::GetFluidBox, ConnectionFactory, [&](auto& Scope, UFGPipeConnectionFactory* Connection) { FFluidBox* Box = Scope(Connection); if(ensureAlways(Box)) { Box->FlowLimit = mFlowLimit; } } )

	SUBSCRIBE_METHOD_VIRTUAL( AFGBuildablePipeReservoir::GetFluidBox, BuildablePipeReservoir, [&](auto& Scope, AFGBuildablePipeReservoir* Reservoir) { FFluidBox* Box = Scope(Reservoir); if(ensureAlways(Box)) { Box->FlowLimit = mFlowLimit; } } )

	SUBSCRIBE_METHOD_VIRTUAL( AFGBuildablePipelineAttachment::GetFluidBox, PipelineJunction, [&](auto& Scope, AFGBuildablePipelineAttachment* PipelineAttachment) { FFluidBox* Box = Scope(PipelineAttachment); if(ensureAlways(Box)) { Box->FlowLimit = mFlowLimit; } } )

	SUBSCRIBE_METHOD_VIRTUAL( AFGBuildablePipeline::GetFluidBox, ConnectionFactory, [&](auto& Scope, AFGBuildablePipeline* Pipeline) { if(ensureAlways(Pipeline)) { FFluidBox* Box = Scope(Pipeline); if(ensure(Box)) { Box->FlowLimit = mFlowLimit;
	                          //Box->MaxOverfillPct = mMaxOverfillPct;
	                          Scope.Override(Box); } } } )

	/*
	SUBSCRIBE_METHOD(UFGPipeConnectionComponent::Factory_Internal_PullPipeInput, [&](auto& Scope, UFGPipeConnectionComponent* Connection, float dt, FInventoryStack& out_stack, TSubclassOf< UFGItemDescriptor > type, int32 maxQuantity)
	{
		bool Value = Scope(Connection, dt, out_stack, type, maxQuantity == 200 ? 334 : maxQuantity);

		if(UFGPipeConnectionFactory* FactoryCon = Cast<UFGPipeConnectionFactory>(Connection))
		{
			FactoryCon->mFluidBox.FlowLimit = mFlowLimit;
			//FactoryCon->mFluidBox.MaxOverfillPct = mMaxOverfillPct;
		}
		
		Scope.Override(Value);
	})

	SUBSCRIBE_METHOD(UFGPipeConnectionComponent::Factory_Internal_PushPipeOutput, [&](auto& Scope, UFGPipeConnectionComponent* Connection, float dt, const FInventoryStack& stack)
	{
		int32 Value = Scope(Connection, dt, stack);
		
		if(UFGPipeConnectionFactory* FactoryCon = Cast<UFGPipeConnectionFactory>(Connection))
		{
			FactoryCon->mFluidBox.FlowLimit = mFlowLimit;
			//FactoryCon->mFluidBox.MaxOverfillPct = mMaxOverfillPct;
		}
		
		Scope.Override(Value);
	})*/
}

void UMLMGameWorldModule::SetupSettings()
{
	fgcheck( mFactorySettings );
	UFGFactorySettings* Settings = mFactorySettings.GetDefaultObject();

	UE_LOG( LogTemp, Error, TEXT("FluidTEST: mFluidToInventoryStackRate = %d"), Settings->mFluidToInventoryStackRate )
	UE_LOG( LogTemp, Error, TEXT("FluidTEST: mInventoryStackToFluidRate = %d"), Settings->mInventoryStackToFluidRate )
	UE_LOG( LogTemp, Error, TEXT("FluidTEST: mAddedPipeProductionPressure = %f"), Settings->mAddedPipeProductionPressure )
	UE_LOG( LogTemp, Error, TEXT("FluidTEST: mViscosityToPuddlePairs = %d"), Settings->mViscosityToPuddlePairs.Num() )
}
