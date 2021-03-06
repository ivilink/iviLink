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









#ifndef CPMPPROTOTHREAD_HPP_
#define CPMPPROTOTHREAD_HPP_

#include "utils/threads/CThread.hpp"
#include "CPmpProtocol.hpp"

namespace iviLink
{

   namespace PMP
   {

      /**
       * This class is used to organize PMP Protocol's own thred
       */
      class CPmpProtoThread: public CThread
      {
      public:
         /**
          * Constructor
          * @param protocol in pointer to PMP Protocol instance
          */
         CPmpProtoThread(CPmpProtocol * protocol);

         /**
          * Destructor
          */
         virtual ~CPmpProtoThread();

         /**
          * disconnects from Channel Supervisor
          */
         void disconnect();

      private:
         /**
          * Thread begins in this function
          */
         virtual void threadFunc();

         CPmpProtocol * mpProtocol; ///< Pointer to PMP Protocol class instance
      };

   }

}

#endif /* CPMPPROTOTHREAD_HPP_ */
