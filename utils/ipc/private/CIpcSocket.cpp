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









#include <algorithm>
#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <limits>
#include <sys/socket.h>
#include <unistd.h>

#include "CIpcSocket.hpp"

struct MsgHeader
{
   UInt32 size;
   UInt32 msg_id;
   UInt32 flags;

   UInt32 isRequest() const
   {
      return flags & 1;
   }
   void setRequest()
   {
      flags |= 1;
   }
   void clearReuest()
   {
      flags &= ~1;
   }
};

const UInt32 MSG_HEADER_SIZE = sizeof(MsgHeader);

///


namespace iviLink {
namespace Ipc {

class DirectionIdGen
{
   static DirectionID count;
   static CMutex mutex;
public:

   static DirectionID next() 
   {
      mutex.lock(); 
      DirectionID d = ++count;
      mutex.unlock();
      return d; 
   }
};
DirectionID DirectionIdGen::count = 0;
CMutex DirectionIdGen::mutex;

Logger CIpcSocket::logger = Logger::getInstance(LOG4CPLUS_TEXT("utils.ipc.CIpcSocket"));

CIpcSocket::CIpcSocket(CIpc& ipc, bool listen) :
   CThread("CIpcReceiver", true),
   mIpc(ipc),
   mListen(listen),
   mContinuousListen(false),
   mLsnSocket(-1),
   mIsConnected(false),
   mMode(RECV_HEADER),
   mMsgID(0),
   mUnknownResponse(true),
   mpCurrentBuf(NULL),
   mpCurrentBufBegin(NULL),
   mHaveInBuffer(0),
   mLeftToReceive(0)
{
   LOG4CPLUS_TRACE(logger, "CIpcSocket()");

   mReadingSocket.sock = -1;
   mReadingSocket.dirId = -1;
   mSelfpipe[0] = mSelfpipe[1] = -1;
   setupSelfpipe();
}

CIpcSocket::~CIpcSocket()
{
   LOG4CPLUS_TRACE(logger, "~CIpcSocket()");
   signalSelfpipe();

   join();

   this->join();

   destroySelfpipe();
}

void CIpcSocket::stopOperations()
{
   LOG4CPLUS_TRACE(logger, "stopOperations()");
   signalSelfpipe();
}

bool CIpcSocket::isConnected() const
{
   bool res = false;
   mSocketMutex.lockRead();
   res = mIsConnected;
   mSocketMutex.unlock();
   return res;
}

bool CIpcSocket::checkThread() const
{
   return !isThisThread();
}

CError CIpcSocket::connect()
{
   LOG4CPLUS_TRACE(logger, "connect()");

   if (isConnected())
      return CIpcError::NoIpcError("Already connected");

   if (!prepareSocketPath())
   {
      return CIpcError(CIpcError::ERROR_COMMUNICATION, "addr prepare");
   }

   sockaddr const* addr = reinterpret_cast<sockaddr const*>(&mAddress);
   const socklen_t addrLen = sizeof(mAddress);

   CError err = CIpcError::NoIpcError();

   mSocketMutex.lockWrite();
   {
      int sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if (-1 == sock)
      {
         err = CIpcError(CIpcError::ERROR_COMMUNICATION, CError::FormErrnoDescr(errno));
      }
      else
      {
         fcntl(sock, F_SETFD, FD_CLOEXEC);

         if (mListen)
         {
            mLsnSocket = sock;
            err = listen();
            if (err.isNoError())
            {
               // not connected yet
               start();
            }
         }
         else
         {
            int res = ::connect(sock, addr, addrLen);
            if (-1 == res)
            {
               err = CIpcError(CIpcError::ERROR_COMMUNICATION, CError::FormErrnoDescr(errno));
            }
            else
            {
               CSockInfo s = { sock, DirectionIdGen::next() };
               mSockets.push_back(s);

               LOG4CPLUS_TRACE(logger, "connect socket = " + convertIntegerToString(s.sock)
            		                 + ", cli = " + convertIntegerToString(s.dirId));

               mIsConnected = true;

               mIpc.onConnectSuccess(s.dirId);

               start();
            }
         }
      }
   }
   mSocketMutex.unlock();

   return err;
}

CError CIpcSocket::setContinuousListen()
{
   mContinuousListen = true;
   return CIpcError::NoIpcError();
}

CError CIpcSocket::stopContinuousListen()
{
   mContinuousListen = false;
   CError err = signalSelfpipe(SPS_END_CONT_LSN);
   return err;
}

bool CIpcSocket::prepareSocketPath()
{
   LOG4CPLUS_TRACE(logger, "prepareSocketPath()");
   memset(&mAddress, 0, sizeof(mAddress));

   const bool isAbstract = true;
   std::string const& addr = mIpc.getAddress();

   if (addr.size() > sizeof(mAddress.sun_path) + 2)
      return false;

   LOG4CPLUS_TRACE(logger, "addr = '" +  addr + "'");
   mAddress.sun_family = AF_UNIX;
   if (isAbstract)
   {
      strcpy(mAddress.sun_path+1, addr.c_str());
   }
   else
   {
      strcpy(mAddress.sun_path, addr.c_str());
   }

   return true;
}

CError CIpcSocket::listen()
{
   LOG4CPLUS_TRACE(logger, "listen()");
   sockaddr const* addr = reinterpret_cast<sockaddr const*>(&mAddress);
   const socklen_t addrLen = sizeof(mAddress);

   if (bind(mLsnSocket, addr, addrLen) == -1)
   {
      return CIpcError(CIpcError::ERROR_COMMUNICATION, CError::FormErrnoDescr(errno));
   }

   if (::listen(mLsnSocket, 1) == -1)
   {
      return CIpcError(CIpcError::ERROR_COMMUNICATION, CError::FormErrnoDescr(errno));
   }

   return CIpcError::NoIpcError();
}

void CIpcSocket::acceptConnection()
{
   LOG4CPLUS_TRACE(logger, "acceptConnection()");

   mSocketMutex.lockWrite();
   {
      int newSock = ::accept(mLsnSocket, NULL, NULL);
      if (newSock != -1)
      {
         LOG4CPLUS_DEBUG(logger, "CIpcSocket::acceptConnection() lsn " + convertIntegerToString(mLsnSocket)
                              + " new_sock " + convertIntegerToString(newSock));
         if (!mContinuousListen)
         {
            closeListenSocket(false);
         }

         fcntl(newSock, F_SETFD, FD_CLOEXEC);

         CSockInfo s = { newSock, DirectionIdGen::next() };
         mSockets.push_back(s);

         mIsConnected = true;

         mIpc.onListenSuccess(s.dirId);
      }
      else
      {
         LOG4CPLUS_WARN(logger, CError::FormErrnoDescr(errno).c_str());
      }
   }
   mSocketMutex.unlock();
}

void CIpcSocket::closeAllSockets()
{
   LOG4CPLUS_TRACE(logger, "closeAllSockets()");

   mSocketMutex.lockWrite();
   {
      closeListenSocket(false);

      for (tSockets::iterator it = mSockets.begin(); it != mSockets.end(); ++it)
      {
         CSockInfo& sock = *it;

         closeSocket(sock.sock);

         mIpc.onConnectionLost(sock.dirId);
      }

      mSockets.clear();
   }
   mSocketMutex.unlock();
}

bool CIpcSocket::closeClientSocket(int sock)
{
   LOG4CPLUS_TRACE(logger, "closeClientSocket()");

   DirectionID dirId = -1;
   bool empty = false;

   mSocketMutex.lockWrite();
   {
      tSockets::iterator it = mSockets.begin();
      for (; it != mSockets.end(); ++it)
      {
         if ((*it).sock == sock)
            break;
      }

      if (it == mSockets.end())
      {
         assert(false);
      }
      dirId = (*it).dirId;

      mSockets.erase(it);

      empty = mSockets.empty();
   }
   mSocketMutex.unlock();

   mIpc.onConnectionLost(dirId);
   closeSocket(sock);

   return empty;
}

void CIpcSocket::closeListenSocket(bool needLock)
{
   if (needLock)
      mSocketMutex.lockWrite();

   if (mLsnSocket != -1)
   {
      closeSocket(mLsnSocket);
      
      mLsnSocket = -1;
      mListen = false;
   }

   if (needLock)
      mSocketMutex.unlock();
}

void CIpcSocket::closeSocket(int sock)
{
   LOG4CPLUS_TRACE(logger, "closeSocket()");
   LOG4CPLUS_TRACE(logger, "sock " + convertIntegerToString(sock));

   int res = ::close(sock);
   if (res == -1)
   {
      /// @todo handle error
      LOG4CPLUS_INFO(logger, "todo handle error");
      LOG4CPLUS_ERROR(logger, CError::FormErrnoDescr(errno).c_str());
   }
}

CError CIpcSocket::setupSelfpipe()
{
   LOG4CPLUS_TRACE(logger, "setupSelfpipe()");
   int res = pipe(mSelfpipe);
   if (-1 == res)
   {
      return CIpcError(CIpcError::ERROR_OTHER, strerror(errno));
   }
   /// @todo error handling in fcntl
   //fcntl(mSelfpipe[0], F_SETFL, fcntl(mSelfpipe[0], F_GETFL) | O_NONBLOCK);
   fcntl(mSelfpipe[1], F_SETFL, fcntl(mSelfpipe[1], F_GETFL) | O_NONBLOCK);

   fcntl(mSelfpipe[0], F_SETFD, FD_CLOEXEC);
   fcntl(mSelfpipe[1], F_SETFD, FD_CLOEXEC);

   return CIpcError::NoIpcError();
}

void CIpcSocket::destroySelfpipe()
{
   LOG4CPLUS_TRACE(logger, "destroySelfpipe()");
   /// @todo error handling
   close(mSelfpipe[0]);
   close(mSelfpipe[1]);

   mSelfpipe[0] = mSelfpipe[1] = -1;
}

CError CIpcSocket::signalSelfpipe(SELFPIPE_SIGNALS sig/* = SPS_ENDALL*/)
{
   LOG4CPLUS_TRACE(logger, "signalSelfpipe()");
   CError err = CIpcError::NoIpcError();
   ssize_t res = -1;

   char buf[1];
   buf[0] = static_cast<char>(sig);
   do {
      res = write(mSelfpipe[1], buf, 1);
   } while (res == -1 && errno == EINTR);

   if (res == -1)
   {
      err = CIpcError(CIpcError::ERROR_OTHER, strerror(errno));
   }

   return err;
}

CIpcSocket::SELFPIPE_SIGNALS CIpcSocket::recvSelfpipeSignal()
{
   LOG4CPLUS_TRACE(logger, "recvSelfpipeSignal()");
   ssize_t res = -1;

   char buf[1] = {0};
   do {
      res = read(mSelfpipe[0], buf, 1);
   } while (res == -1 && errno == EINTR);

   if (res == -1)
   {
      /// @todo error handling
      LOG4CPLUS_ERROR(logger, CError::FormErrnoDescr(errno).c_str());
   }

   return static_cast<SELFPIPE_SIGNALS>(buf[0]);
}

void CIpcSocket::threadFunc()
{
   LOG4CPLUS_TRACE(logger, "threadFunc()");
   fd_set* const rSet = &mRFdSet;

   bool needExit = false;
   int lastReadingSock = -1;

   while (true)
   {
      LOG4CPLUS_DEBUG(logger, "CIpcSocket::threadFunc() iteration");
      bool sockSet = false;
      bool lsnSet  = false;
      bool pipeSet = false;

      mSocketMutex.lockRead();
      {
         FD_ZERO(rSet);

         int maxFD = -1;

         //FD_SET(mSocket,      rSet);
         FD_SET(mSelfpipe[0], rSet);
         maxFD = std::max(mSelfpipe[0], maxFD);

         if (mLsnSocket != -1)
         {
            LOG4CPLUS_DEBUG(logger, "Setting mLsnSocket = " + convertIntegerToString(mLsnSocket));
            FD_SET(mLsnSocket, rSet);
            maxFD = std::max(mLsnSocket, maxFD);
         }

         if (mReadingSocket.sock != -1)
         {
            LOG4CPLUS_DEBUG(logger, "Setting mReadingSocket = " + convertIntegerToString(mReadingSocket.sock)
                                 + ", dir = " + convertIntegerToString(mReadingSocket.dirId));
            FD_SET(mReadingSocket.sock, rSet);
            maxFD = std::max(mReadingSocket.sock, maxFD);
         }
         else
         {
            for (tSockets::const_iterator it = mSockets.begin(); it != mSockets.end(); ++it)
            {
               CSockInfo const& sock = *it;
               LOG4CPLUS_DEBUG(logger, "Setting socket " + convertIntegerToString(sock.sock)
            		                + " dir = " + convertIntegerToString(sock.dirId));
               FD_SET(sock.sock, rSet);
               maxFD = std::max(sock.sock, maxFD);
            }
         }

         // Must be max+1
         ++maxFD;

         int res = -1;
         do
         {
            LOG4CPLUS_DEBUG(logger, "before select maxFD = " + convertIntegerToString(maxFD));
            res = ::select(maxFD, rSet, NULL, NULL, NULL);
         } while (res == -1 && errno == EINTR);
         LOG4CPLUS_DEBUG(logger, "select res = " + convertIntegerToString(res));

         if (res <= -1)
         {
            /// @todo error handling
            needExit = true;
         }
         else
         {
            pipeSet = FD_ISSET(mSelfpipe[0], rSet);

            if (mLsnSocket != -1)
            {
               lsnSet = FD_ISSET(mLsnSocket, rSet);
            }

            if (mReadingSocket.sock != -1)
            {
               sockSet = FD_ISSET(mReadingSocket.sock, rSet);
            }
            else
            {
               for (tSockets::const_iterator it = mSockets.begin(); it != mSockets.end(); ++it)
               {
                  CSockInfo const& sock = *it;
                  if (FD_ISSET(sock.sock, rSet))
                  {
                     // Memorizing reading socket so all consumeData operations
                     // will be from this socket
                     mReadingSocket = sock;
                     lastReadingSock = mReadingSocket.sock;
                     sockSet = true;
                     break;
                  }
               }
            }
         } // if res < -1
      }
      mSocketMutex.unlock();

      if (needExit)
         break;

      if (pipeSet)
      {
         switch (recvSelfpipeSignal())
         {
         case SPS_ENDALL:
            needExit = true;
            break;
         case SPS_END_CONT_LSN:
            closeListenSocket(true);
            break;
         case SPS_CLI_DIR_CLOSED:
            {
               int closedSock = -1;
               mClosedSocketsQueueMutex.lock();
               if (mClosedSocketsQueue.empty())
               {
                  LOG4CPLUS_ERROR(logger, "mClosedSocketsQueue is empty which is wrong, stopping");
                  needExit = true;
               }
               else
               {
                  closedSock = mClosedSocketsQueue.front();
                  mClosedSocketsQueue.pop();
                  LOG4CPLUS_DEBUG(logger, "closed direstion sock = " + convertIntegerToString(closedSock));
               }
               mClosedSocketsQueueMutex.unlock();

               if (closedSock != -1)
               {
                  if (lastReadingSock == closedSock)
                     setMode(RECV_HEADER, MSG_HEADER_SIZE);

                  bool last = closeClientSocket(closedSock);
                  LOG4CPLUS_DEBUG(logger, "is last sock = " + convertIntegerToString((int)last));

                  needExit = last && !mListen;
               }
            }
            break;
         }

         if (needExit)
            break;

         continue;
      }

      if (lsnSet)
      {
         acceptConnection();

         continue;
      }

      if (sockSet && !consumeData())
      {
         bool last = closeClientSocket(lastReadingSock);
         LOG4CPLUS_DEBUG(logger, "is last sock = " + convertIntegerToString((int)last));

         if (last && !mListen)
            break;

         setMode(RECV_HEADER, MSG_HEADER_SIZE);

         continue;
      }

   } // while

   closeAllSockets();
}


bool CIpcSocket::consumeData()
{
   LOG4CPLUS_TRACE(logger, "consumeData()");

   switch (mMode)
   {
   case RECV_HEADER:
      return consumeHeader();
   case RECV_DATA_REQU:
      return consumeRequestData();
   case RECV_DATA_RESP:
      return consumeResponseData();
   default:
      return false;
   }
}

bool CIpcSocket::consumeHeader()
{
   LOG4CPLUS_TRACE(logger, "consumeHeader()");

   const UInt32 headerSize = sizeof(MsgHeader);

   assert(mHaveInBuffer < headerSize);
   UInt32 size = headerSize - mHaveInBuffer;

   CError err = receive(mBuffer + mHaveInBuffer, size);

   if (!err.isNoError())
   {
      LOG4CPLUS_ERROR(logger, ((std::string)err).c_str());
      return false;
   }

   assert(mHaveInBuffer + size <= headerSize);

   if (mHaveInBuffer + size < headerSize)
   {
      // header is not ready
      mHaveInBuffer += size;
      return true;
   }

   // header ready

   MsgHeader const* const hdr = reinterpret_cast<MsgHeader const*>(mBuffer);

   mMsgID = hdr->msg_id;

   LOG4CPLUS_DEBUG(logger, "got header id " + convertIntegerToString(mMsgID)
                                + ", req " + convertIntegerToString((int)hdr->isRequest())
                                 + ", ps " + convertIntegerToString(hdr->size));

   if (hdr->isRequest())
   {
      setMode(RECV_DATA_REQU, hdr->size);

      /// @todo assign buffer
      assert(BUFFER_SIZE >= mLeftToReceive);
      mpCurrentBufBegin = mpCurrentBuf = mBuffer;
      LOG4CPLUS_DEBUG(logger, "header request setting buf = " + convertIntegerToString(*mpCurrentBuf));
   }
   else
   {
      setMode(RECV_DATA_RESP, hdr->size);

      CIpc::RequestData data;

      mUnknownResponse = !mIpc.mRequestDataContainer.getData(mMsgID, data);

      if (!mUnknownResponse && mLeftToReceive > data.bufferSize)
      {
         mUnknownResponse = true;
         /// @todo message about small buffer
         LOG4CPLUS_DEBUG(logger, "buffer is too small left " + convertIntegerToString(mLeftToReceive)
                                              + " bufSize " + convertIntegerToString(data.bufferSize));
      }

      if (mUnknownResponse)
      {
         mpCurrentBufBegin = mpCurrentBuf = mBuffer;
         LOG4CPLUS_DEBUG(logger, "mUnknownResponse");
      }
      else
      {
         mpCurrentBufBegin = mpCurrentBuf = data.pResponseBuffer;
      }

      // In case of empty response
      if (mLeftToReceive == 0)
      {
         bool res = CIpcSocket::consumeResponseData();
         assert(res);
      }
   }

   LOG4CPLUS_DEBUG(logger, "mMsgID " + convertIntegerToString(mMsgID)
              + ", mLeftToReceive " + convertIntegerToString(mLeftToReceive)
               + ", mHaveInBuffer " + convertIntegerToString(mHaveInBuffer));

   return true;
}

bool CIpcSocket::consumeRequestData()
{
   LOG4CPLUS_TRACE(logger, "consumeRequestData()");

   UInt32 size = mLeftToReceive;

   int recvRes = recvReqRespData(size);

   if (recvRes > 0)
   {
      // data ready
      LOG4CPLUS_DEBUG(logger, "request done data in buf " + convertIntegerToString(mHaveInBuffer));

      mIpc.incomingRequest(mMsgID, mpCurrentBufBegin, mHaveInBuffer, mReadingSocket.dirId);
   }

   if (recvRes != 0)
   {
      // Request ended or socket closed
      setMode(RECV_HEADER, MSG_HEADER_SIZE);
   }

   return recvRes >= 0;
}

bool CIpcSocket::consumeResponseData()
{
   LOG4CPLUS_TRACE(logger, "consumeResponseData()");

   UInt32 size = mLeftToReceive;

   CIpc::RequestData* data = NULL;
   if (!mUnknownResponse && !mIpc.mRequestDataContainer.lockData(mMsgID, data))
   {
      // Response buffer has been deleted. Probably, timeout
      mUnknownResponse = true;
      LOG4CPLUS_DEBUG(logger, "mUnknownResponse");
   }

   if (mUnknownResponse)
   {
      // We need to read packet data from socket in this internal buffer
      mpCurrentBufBegin = mpCurrentBuf = mBuffer;
   }

   bool res = false;
   int recvRes = 1;
   if (size > 0)
   {
      // Response is not empty, need to read data from socket
      recvRes = recvReqRespData(size);
      LOG4CPLUS_DEBUG(logger, "recvReqRespData = " + convertIntegerToString(recvRes));
   }

   if (recvRes > 0)
   {
      // data is ready
      if (!mUnknownResponse)
      {
         LOG4CPLUS_DEBUG(logger, "signaling about data in buf " + convertIntegerToString(mHaveInBuffer));
         data->bufferSize = mHaveInBuffer;
         data->recvSem->signal();
      }
      else
      {
         LOG4CPLUS_INFO(logger, "Ipc got unknown response MsgId=" + convertIntegerToString(mMsgID));
      }

      res = true;
   }
   else if (recvRes < 0)
   {
      res =  false;
   }
   else // (recvRes == 0)
   {
      res = true;
   }

   if (recvRes != 0)
   {
      // Response ended or socket closed
      setMode(RECV_HEADER, MSG_HEADER_SIZE);
   }

   if (!mUnknownResponse)
   {
      mIpc.mRequestDataContainer.unlockData(mMsgID);
   }

   return res;
}

int CIpcSocket::recvReqRespData(UInt32& size)
{
   LOG4CPLUS_TRACE(logger, "recvReqRespData");

   CError err = receive(mpCurrentBuf, size);
   if (!err.isNoError())
   {
      /// @todo log, error handling
      LOG4CPLUS_INFO(logger, "todo: add handling");
      LOG4CPLUS_ERROR(logger, ((std::string)err).c_str());
      return -1;
   }

   LOG4CPLUS_DEBUG(logger, "size " + convertIntegerToString(size)
             + " mLeftToReceive " + convertIntegerToString(mLeftToReceive)
              + " mHaveInBuffer " + convertIntegerToString(mHaveInBuffer));

   assert(size <= mLeftToReceive);
   assert(mHaveInBuffer + size <= mLeftToReceive);

   int res = 1;

   if (mHaveInBuffer + size < mLeftToReceive)
   {
      // data is not ready
      res = 0;
   }

   mpCurrentBuf += size;
   mHaveInBuffer += size;
   mLeftToReceive -= size;

   LOG4CPLUS_DEBUG(logger, "size " + convertIntegerToString(size)
             + " mLeftToReceive " + convertIntegerToString(mLeftToReceive)
              + " mHaveInBuffer " + convertIntegerToString(mHaveInBuffer));

   return res;
}

CError CIpcSocket::receive(UInt8* pBuffer, UInt32& bufferSize, bool peek/* = false*/)
{
   LOG4CPLUS_TRACE(logger, "receive()");

   ssize_t n = -1;
   int savedErrno = 0;

   mSocketMutex.lockRead();
   {
      // Internal cycle must be made so mSocket is exist

      assert(mReadingSocket.sock != -1);
      do {
         n = ::recv(mReadingSocket.sock, pBuffer, bufferSize, peek ? MSG_PEEK : 0);
      } while (n == -1 && errno == EINTR);

      savedErrno = errno;
   }
   mSocketMutex.unlock();

   if (n > 0)
   {
      //hex_print(pBuffer, n);
      bufferSize = n;
      return CIpcError::NoIpcError();
   }
   else if (n == 0)
   {
      return CIpcError(CIpcError::ERROR_CONNECTION_LOST, "ERROR_CONNECTION_LOST");
   }
   else
   {
      return CIpcError(CIpcError::ERROR_COMMUNICATION, strerror(savedErrno));
   }

   return CIpcError::NoIpcError();
}


CError CIpcSocket::send(MsgID id, bool isRequest, UInt8 const* pPayload, UInt32 payloadSize, DirectionID const * const pDirId)
{
   LOG4CPLUS_TRACE(logger, "send()");

   LOG4CPLUS_DEBUG(logger, "id " + convertIntegerToString(id)
                     + ", req " + convertIntegerToString((int)isRequest)
                      + ", ps " + convertIntegerToString(payloadSize));

   if (!isConnected())
      return CIpcError(CIpcError::ERROR_CONNECTION_LOST, "ERROR_CONNECTION_LOST");

   MsgHeader header;

   //assert(payloadSize < std::numeric_limits<UInt32>::max() - sizeof(header));
   if (!(payloadSize < std::numeric_limits<UInt32>::max() - sizeof(header)))
      return CIpcError(CIpcError::ERROR_INVALID_PARAMS, "Buffer is too big");

   header.msg_id = id;
   header.size = payloadSize;
   header.flags = 0;
   if (isRequest)
      header.setRequest();

   CError err = CIpcError::NoIpcError();
   mSocketMutex.lockRead();
   {
      //hex_print(&header, sizeof(header));

      ssize_t res = -1;

      int sock = findClientSocket(pDirId);

      /// @todo timedLoc
      mSendMutex.lock();
      {
         if (sock == -1)
         {
            err = CIpcError(CIpcError::ERROR_UNKNOWN_DIRECTION, "Direction with specified dirId is unknown");
            LOG4CPLUS_ERROR(logger, ((std::string)err).c_str());
         }
         else
         {
            do {
               LOG4CPLUS_DEBUG(logger, "send sock " + convertIntegerToString(sock)
                                         + " dir " + convertIntegerToString(pDirId ? *pDirId : -1));
               res = ::send(sock, &header, sizeof(header), MSG_NOSIGNAL);
            } while (res == -1 && errno == EINTR);
         }

         if (-1 != res && err.isNoError() && payloadSize > 0)
         {
            do {
               LOG4CPLUS_DEBUG(logger, "send sock " + convertIntegerToString(sock)
                                         + " dir " + convertIntegerToString(pDirId ? *pDirId : -1));
               res = ::send(sock, pPayload, payloadSize, MSG_NOSIGNAL);
            } while (res == -1 && errno == EINTR);
         }

         if (-1 == res)
         {
            int saved_errno = errno;

            mClosedSocketsQueueMutex.lock();
            mClosedSocketsQueue.push(sock);
            mClosedSocketsQueueMutex.unlock();

            signalSelfpipe(SPS_CLI_DIR_CLOSED);

            err = CIpcError(CIpcError::ERROR_CONNECTION_LOST, CError::FormErrnoDescr(saved_errno));
            LOG4CPLUS_ERROR(logger, ((std::string)err).c_str());
         }

      }
      mSendMutex.unlock();

   }
   mSocketMutex.unlock();

   return err;
}

int CIpcSocket::findClientSocket(DirectionID const * const pDirId) const
{
   if (pDirId)
   {
      for (tSockets::const_iterator it = mSockets.begin(); it != mSockets.end(); ++it)
      {
         if ((*it).dirId == *pDirId)
            return (*it).sock;
      }
   }
   else
   {
      if (!mSockets.empty())
         return mSockets[0].sock;
   }

   return -1;
}

void CIpcSocket::setMode(RECV_MODE mode, UInt32 leftToReceive)
{
   LOG4CPLUS_TRACE(logger, "CIpcSocket::setMode " + convertIntegerToString((int)mode));

   mMode = mode;

   if (mode == RECV_HEADER)
   {
      mReadingSocket.sock = -1;
   }

   mLeftToReceive = leftToReceive;
   mHaveInBuffer = 0;
}

}  // namespace Ipc
}  // namespace AXIS

