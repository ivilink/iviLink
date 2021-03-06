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









#ifndef CADAPTERFACTORY_HPP_
#define CADAPTERFACTORY_HPP_

/********************************************************************
 *
 * Other includes
 *
 ********************************************************************/
#include "utils/misc/Types.hpp"
#include "CCarrierAdapter.hpp"
#include "ConnectionInfo.hpp"
/********************************************************************
 *
 * Forward declarations
 *
 ********************************************************************/

namespace iviLink
{
   namespace ConnectivityAgent
   {
      namespace HAL
      {

         class CTcpCarrierAdapter;

         /// CAdapterFactory class
         /**
          * This class is used to encapsulate creation of carrier adapters
          */
         class CAdapterFactory
            {
         public:
            // Methods section

            /**
             * Factory method for CTcpCarrierAdapter
             * @param connectionInfo - information for TCP connection establishing
             * @return instance of CTcpCarrierAdapter
             */
            static CTcpCarrierAdapter* CreateTcpAdapter(const CTcpConnectionInfo & connectionInfo);

         private:
            // Methods section

            /**
             * Default constructor.
             * Unimplemented to avoid misusing.
             */
            CAdapterFactory();

            /**
             * Copying constructor.
             * Unimplemented to avoid misusing.
             * @param classRef The reference to the object the class is constructing on.
             */
            CAdapterFactory(const CAdapterFactory& classRef);

            /**
             * Default destructor.
             * Unimplemented to avoid misusing.
             */
            ~CAdapterFactory();

            /**
             * Assigning operator.
             * Unimplemented to avoid misusing.
             * @param classRef The reference to the object the class is assigning from.
             */
            CAdapterFactory& operator=(const CAdapterFactory& classRef);

         };
      }
   }
}
#endif /* CADAPTERFACTORY_HPP_ */
