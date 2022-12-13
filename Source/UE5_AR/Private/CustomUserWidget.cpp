// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUserWidget.h"

void UCustomUserWidget::SetUIValue(const FString& Key, const FString& Value, TEnumAsByte<ValueType> Type)
{
	FUIBufferData Data;
	Data.Data = Value;
	Data.Type = Type;

	if (Buffer.Contains(Key))
		Buffer[Key] = Data;
	else
		Buffer.Add(Key, Data);
}

bool UCustomUserWidget::ContainsUIValue(const FString& Key) const
{
	return Buffer.Contains(Key);
}

ValueType UCustomUserWidget::GetUIValueType(const FString& Key) const
{
	if (Buffer.Contains(Key))
		return Buffer[Key].Type;

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("No Value at: %s"), *Key));
	return NaN;
}

int UCustomUserWidget::GetUIValueAsInt(const FString& Key) const
{
	if (Buffer.Contains(Key))
		return FCString::Atoi(*Buffer[Key].Data);

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("No Value at: %s"), *Key));
	return 0;
}

FString UCustomUserWidget::GetUIValueAsString(const FString& Key) const
{
	if (Buffer.Contains(Key))
		return Buffer[Key].Data;

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("No Value at: %s"), *Key));
	return FString();
}

float UCustomUserWidget::GetUIValueAsFloat(const FString& Key) const
{
	if (Buffer.Contains(Key))
		return FCString::Atof(*Buffer[Key].Data);

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("No Value at: %s"), *Key));
	return 0.f;
}

double UCustomUserWidget::GetUIValueAsDouble(const FString& Key) const
{
	if (Buffer.Contains(Key))
		return FCString::Atod(*Buffer[Key].Data);

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("No Value at: %s"), *Key));
	return 0.f;
}

int32 UCustomUserWidget::GetUIValueAsPtr(const FString& Key) const
{
	return GetUIValueAsInt(Key);
}
