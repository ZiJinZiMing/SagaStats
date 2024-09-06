/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SSDelegates.h"

FSSDelegates::FSSOnVariableAddedOrRemoved FSSDelegates::OnVariableAdded;
FSSDelegates::FSSOnVariableAddedOrRemoved FSSDelegates::OnVariableRemoved;
FSSDelegates::FSSOnVariableRenamed FSSDelegates::OnVariableRenamed;
FSSDelegates::FSSOnVariableTypeChanged FSSDelegates::OnVariableTypeChanged;
FSSDelegates::FSSOnPreCompile FSSDelegates::OnPreCompile;
FSSDelegates::FSSOnPostCompile FSSDelegates::OnPostCompile;
FSSDelegates::FSSOnRequestDetailsRefresh FSSDelegates::OnRequestDetailsRefresh;
