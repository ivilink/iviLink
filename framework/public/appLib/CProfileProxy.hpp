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




 
/**
 * \file CProfileProxy.hpp
 * This header contains declaration of the base class
 * for all profile proxies. The class shoudln't be inherited directly,
 * macros from macro.hpp should be used instead.
 * (see samples/linux/Profiles/ProfileProxy/CBasicSampleProfileProxy.hpp)
 */


#ifndef CPROFILEHANDLER_HPP_
#define CPROFILEHANDLER_HPP_

#include "utils/misc/UID.hpp"
#include "macro.hpp"
#include "framework/public/appLib/CProfileApiBase.hpp"

namespace iviLink
{
   namespace App
   {
      template <typename ProfileApi>
      class CProfileProxy;

      class CProfileGetter
      {
         template <typename ProfileApi> friend class CProfileProxy;
         static CProfileApiBase * getBaseProfile(const Service::Uid & service, const Profile::ApiUid & api);
         static void releaseProfile(const Service::Uid & service, const Profile::ApiUid & api);
      };
      
      
      /** 
       * \class CProfileProxy 
       * A base class
       * for all profile proxies. The class shoudln't be inherited directly,
       * macros from macro.hpp should be used instead.
       * (see samples/linux/Profiles/ProfileProxy/CBasicSampleProfileProxy.hpp).
       */

      template <typename ProfileApi>
      class CProfileProxy
      {
      public:
         CProfileProxy(const Service::Uid & service, const Profile::ApiUid & api)
            : mService(service)
            , mApi(api)
         {
         }
         virtual ~CProfileProxy()
         {
         }

      protected:
         ProfileApi * getProfile() const
         {
            return static_cast<ProfileApi*>(CProfileGetter::getBaseProfile(mService,mApi));
         }

         void releaseProfile() const
         {
            CProfileGetter::releaseProfile(mService, mApi);
         }

      private:

         Service::Uid mService;
         Profile::ApiUid mApi;
      };
   }

} /* namespace iviLink */
#endif /* CPROFILEHANDLER_HPP_ */
