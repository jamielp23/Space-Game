// Copyright Continuum Saga. All Rights Reserved.

#include "ModularWeapons.h"

#define LOCTEXT_NAMESPACE "FModularWeaponsModule"

DEFINE_LOG_CATEGORY(LogModularWeapons);

void FModularWeaponsModule::StartupModule()
{
	UE_LOG(LogModularWeapons, Log, TEXT("ModularWeapons module started."));
}

void FModularWeaponsModule::ShutdownModule()
{
	UE_LOG(LogModularWeapons, Log, TEXT("ModularWeapons module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularWeaponsModule, ModularWeapons)
