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









#ifndef CONNECTIVITY_AGENT_MDG_PROXY_HPP
#define CONNECTIVITY_AGENT_MDG_PROXY_HPP

#include <string>
#include "utils/misc/CError.hpp"
#include "utils/ipc/ICallbackHandler.hpp"
#include "utils/ipc/ipc_common.hpp"
#include "utils/ipc/CIpc.hpp"

#include "utils/misc/Logger.hpp"

using namespace std;
using namespace iviLink::Ipc;

namespace SystemControllerMsgProtocol
{

class ConnectivityAgentMsgProxy : public iviLink::Ipc::ICallbackHandler
{
private:
   static Logger logger;

   const char* getName() {return "ConnectivityAgentMsgProxy";}

   iviLink::Ipc::CIpc* mpIpc;

public:
   explicit ConnectivityAgentMsgProxy(const string connectionName);
   virtual ~ConnectivityAgentMsgProxy();

protected:

   // Outgoing messages
   //
   CError requestShutDown();

   // Incoming messages
   // should be implemented by implementation
   virtual CError onCounterCAConnected(UInt8 role) = 0;
   virtual CError onCounterCADisconnected() = 0;

protected:
   // from ICallbackHandler

   virtual void OnConnection(iviLink::Ipc::DirectionID);
   virtual void OnConnectionLost(iviLink::Ipc::DirectionID);

   virtual void OnRequest(iviLink::Ipc::MsgID id, UInt8 const* pPayload, UInt32 payloadSize, UInt8* const pResponseBuffer, UInt32& bufferSize, iviLink::Ipc::DirectionID);

   virtual void onConnectivityAgentAvailable() = 0;
   virtual void onConnectivityAgentNotAvailable() = 0;

   class CMsgIdGen
   {
   public:
      CMsgIdGen();
      ~CMsgIdGen();
      iviLink::Ipc::MsgID getNext();
   private:
      iviLink::Ipc::MsgID mId;
   };

   CMsgIdGen mMsgIdGen;

   CError connectConnectivityAgent();

   bool isConnected() const;

   enum
   {
      BUFFER_SIZE = 4096
   };

   UInt8 mReadBuffer[BUFFER_SIZE];
   UInt8 mWriteBuffer[BUFFER_SIZE];
};

}
#endif /* CONNECTIVITY_AGENT_MDG_PROXY_HPP */
