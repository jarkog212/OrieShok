// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomUserWidget.generated.h"

//! @brief Enumerator denoting data type
UENUM()
enum ValueType
{
	NaN  UMETA(DisplayName = "Wrong"),
	Int UMETA(DisplayName = "Integer"),
	String UMETA(DisplayName = "String"),
	Float UMETA(DisplayName = "Float"),
	Double UMETA(DisplayName = "Double"),
	Ptr UMETA(DisplayName = "Pointer")
};

//! @brief Structure encapsulating the byte data with the expected data type
USTRUCT()
struct FUIBufferData
{
	GENERATED_BODY()

	//! The data in byte form
	FString Data;

	//! The data type of the data in the byte form
	TEnumAsByte<ValueType> Type;
};

//! @brief Class expanding the default user widget by a data map allowing the Blueprints sending values back to C++
//! [This functionality is unused for now]
UCLASS()
class UE5_AR_API UCustomUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//! @brief Function saving the data an dits type into the data map
	//! @param Key - String key to the data in map.
	//! @param Value - The actual data to be stored in byte form.
	//! @param Type - The data type of the data to be stored.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		void SetUIValue(const FString& Key, const FString& Value, TEnumAsByte<ValueType> Type);

	//! @brief Function that returns whether the map already contains stored data for a given key
	//! @param Key - String key to check for.
	//! @returns true - There is data present in the map for the given Key
	//! @returns false - otherwise.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		bool ContainsUIValue(const FString& Key) const;

	//! @brief Function that returns the data type of the stored data at the given Key
	//! @param Key - String key to the data in map.
	//! @returns Nan - If there is no data present at the given Key.
	//! @returns [value] - The data type of the stored data at the Key location
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		ValueType GetUIValueType(const FString& Key) const;

	//! @brief Function that returns the stored data at Key position
	//! Data is expected to be an integer.
	//! Must be preceded by contain check.
	//! @param Key - String key to the data in map.
	//! @returns [value] - Stored value as an integer.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		int GetUIValueAsInt(const FString& Key) const;

	//! @brief Function that returns the stored data at Key position
	//! Data is expected to be a string.
	//! Must be preceded by contain check.
	//! @param Key - String key to the data in map.
	//! @returns [value] - Stored value as a string.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		FString GetUIValueAsString(const FString& Key) const;

	//! @brief Function that returns the stored data at Key position
	//! Data is expected to be a float.
	//! Must be preceded by contain check.
	//! @param Key - String key to the data in map.
	//! @returns [value] - Stored value as a float.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		float GetUIValueAsFloat(const FString& Key) const;

	//! @brief Function that returns the stored data at Key position
	//! Data is expected to be a double.
	//! Must be preceded by contain check.
	//! @param Key - String key to the data in map.
	//! @returns [value] - Stored value as a double.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		double GetUIValueAsDouble(const FString& Key) const;

	//! @brief Function that returns the stored data at Key position
	//! Data is expected to be a pointer (int32).
	//! Must be preceded by contain check.
	//! @param Key - String key to the data in map.
	//! @returns [value] - Stored value as an int32.
	UFUNCTION(BlueprintCallable, Category = "UI Buffer")
		int32 GetUIValueAsPtr(const FString& Key) const;

private:

	//Hidden

	//! The data map used for holding the data between blueprints and C++
	TMap<FString, FUIBufferData> Buffer;
};
