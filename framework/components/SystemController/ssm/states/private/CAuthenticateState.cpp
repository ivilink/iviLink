/* 
 * 
 * iviLINK SDK, version 1.0.1
 * http://www.ivilink.net
 * Cross Platform Application Communication Stack for In-Vehicle Applications
 * 
 * Copyright (C) 2012, Luxoft Professional Corp., member of IBS group
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2.1.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * 
 * 
 */






#include "framework/components/SystemController/ssm/states/CAuthenticateState.hpp"
#include "framework/components/SystemController/ssm/states/CIdleState.hpp"
#include "framework/components/SystemController/ssm/stateMachine/CSystemStateMachine.hpp"
#include "framework/components/SystemController/componentLauncher/CComponentLauncher.hpp"
#include "framework/components/SystemController/ssm/stateMachine/CTriggerQueue.hpp"

namespace SystemController
{

Logger CAuthenticateState::sLogger = Logger::getInstance(LOG4CPLUS_TEXT("systemController.SystemStateMachine.states.CAuthenticateState"));
CAuthenticateState* CAuthenticateState::sInstance = 0;

CSystemState* CAuthenticateState::getInstance()
{
   if(0 == sInstance)
   {
	   sInstance = new CAuthenticateState;
   }

   return sInstance;
}

CAuthenticateState::CAuthenticateState()
{

}

CAuthenticateState::~CAuthenticateState()
{

}

void CAuthenticateState::startAuthentication(CSystemStateMachine* stateMachine)
{
   LOG4CPLUS_TRACE(sLogger, "startAuthentication()");

   if(stateMachine->authOn)
   {
      stateMachine->connectAuthenticationApp();

      if (stateMachine->mSystemGender == 1)
      {
         CComponentLauncher::getInstance()->launchAuthenticationApp();
      }
   }
   else
   {
      LOG4CPLUS_TRACE(sLogger, "noauth option requested --> Skip authentication");
      changeState(stateMachine, CIdleState::getInstance());
   }
}

void CAuthenticateState::finishAuthentication(CSystemStateMachine* stateMachine)
{
   LOG4CPLUS_TRACE(sLogger, "finishAuthentication()");

   changeState(stateMachine, CIdleState::getInstance());

   CTriggerQueue::sendTrigger(AUTHENTICATION_ESTABLISHED);
}

void CAuthenticateState::cancelAuthentication(CSystemStateMachine* stateMachine)
{
   LOG4CPLUS_TRACE(sLogger, "cancelAuthentication()");

   changeState(stateMachine, CIdleState::getInstance());

   CTriggerQueue::sendTrigger(AUTHENTICATION_CANCELED);
}

} /* namespace SystemController */
