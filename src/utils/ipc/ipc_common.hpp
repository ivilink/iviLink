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









#ifndef IPCLIB_COMMON_HPP_
#define IPCLIB_COMMON_HPP_

#include <string>
#include "utils/misc/Types.hpp"

namespace iviLink {
namespace Ipc {

typedef UInt32 MsgID; ///< Type for message IDs

typedef std::string Address; ///< Type for address

typedef Int32 DirectionID;	///< Type for direction IDs

}  // namespace Ipc
}  // namespace AXIS

#endif /* IPCLIB_COMMON_HPP_ */
