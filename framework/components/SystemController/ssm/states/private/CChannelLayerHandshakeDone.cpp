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







#include "framework/components/SystemController/ssm/stateMachine/CSystemStateMachine.hpp"
#include "framework/components/SystemController/componentLauncher/CComponentLauncher.hpp"
#include "framework/components/SystemController/ssm/stateMachine/CTriggerQueue.hpp"
#include "framework/components/SystemController/ssm/states/CChannelLayerHandshakeDone.hpp"
#include "framework/components/SystemController/ssm/states/CProfileManagerReady.hpp"

namespace SystemController
{

Logger CChannelLayerHandshakeDone::sLogger = Logger::getInstance(LOG4CPLUS_TEXT("systemController.SystemStateMachine.states.CChannelLayerHandshakeDone"));
CChannelLayerHandshakeDone* CChannelLayerHandshakeDone::sInstance = 0;

CSystemState* CChannelLayerHandshakeDone::getInstance()
{
   if(0 == sInstance)
   {
	   sInstance = new CChannelLayerHandshakeDone;
   }

   return sInstance;
}


CChannelLayerHandshakeDone::CChannelLayerHandshakeDone()
{}

CChannelLayerHandshakeDone::~CChannelLayerHandshakeDone()
{}

void CChannelLayerHandshakeDone::launchProfileManager(CSystemStateMachine* stateMachine)
{
   LOG4CPLUS_TRACE(sLogger, "launchProfileManager()");

   //LOG4CPLUS_WARN(sLogger, "Application manager is launched here. Create specific method and probably state for this");
   //CComponentLauncher::getInstance()->launchApplicationManager();

   CComponentLauncher::getInstance()->launchProfileManager();
   stateMachine->connectProfileManager();
}

void CChannelLayerHandshakeDone::onProfileManagerAvailable(CSystemStateMachine* stateMachine)
{
   LOG4CPLUS_TRACE(sLogger, "onProfileManagerAvailable()");

   changeState(stateMachine, CProfileManagerReady::getInstance());

   CTriggerQueue::sendTrigger(PROFILE_MANAGER_STARTED);
}

}

