// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file ListenResourceImpl.cpp
 *
 */

#include "ListenResourceImpl.h"
#include <fastrtps/rtps/resources/ListenResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include "../participant/RTPSParticipantImpl.h"

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/RTPSLog.h>

#define IDSTRING "(ID:"<<this->mp_listenResource->m_ID<<") "<<

using boost::asio::ip::udp;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "ListenResourceImpl";

typedef std::vector<RTPSWriter*>::iterator Wit;
typedef std::vector<RTPSReader*>::iterator Rit;

ListenResourceImpl::ListenResourceImpl(ListenResource* LR):
																								mp_RTPSParticipantImpl(nullptr),
																								mp_listenResource(LR),
																								mp_thread(nullptr),
																								m_listen_socket(m_io_service),
                                                                                                runningAsync_(false), stopped_(false)

{

}

ListenResourceImpl::~ListenResourceImpl()
{
	const char* const METHOD_NAME = "~ListenResourceImpl";
	if(mp_thread !=nullptr)
	{
        boost::unique_lock<boost::mutex> lock(mutex_);
        if(runningAsync_)
            cond_.wait(lock);

		logInfo(RTPS_MSG_IN,IDSTRING"Removing listening thread " << mp_thread->get_id() <<" socket: "
				<<m_listen_socket.local_endpoint() <<  " locators: " << mv_listenLoc,C_BLUE);

        m_listen_socket.cancel();
		m_listen_socket.close();

		m_io_service.stop();
        stopped_ = true;
        lock.unlock();

		logInfo(RTPS_MSG_IN,"Joining with thread",C_BLUE);
		mp_thread->join();

		delete(mp_thread);
		logInfo(RTPS_MSG_IN,"Listening thread closed succesfully",C_BLUE);
	}
}

bool ListenResourceImpl::isListeningTo(const Locator_t& loc)
{
	if(IsAddressDefined(loc))
	{
		LocatorList_t locList = mv_listenLoc;
		return locList.contains(loc);
	}
	else
	{
		if(loc.port == mv_listenLoc.begin()->port)
			return true;
	}
	return false;
}



void ListenResourceImpl::newCDRMessage(const boost::system::error_code& err, std::size_t msg_size)
{
	const char* const METHOD_NAME = "newCDRMessage";

	if(err == boost::system::errc::success)
	{
        boost::unique_lock<boost::mutex> lock(mutex_);
        if(stopped_)
            return;
        runningAsync_ = true;
        lock.unlock();

		mp_listenResource->setMsgRecMsgLength((uint32_t)msg_size);

		if(msg_size == 0)
			return;
		try{
			logInfo(RTPS_MSG_IN,IDSTRING mp_listenResource->mp_receiver->m_rec_msg.length
					<< " bytes FROM: " << m_sender_endpoint << " TO: " << m_listen_socket.local_endpoint(),C_BLUE);

			//Get address into Locator
			m_senderLocator.port = m_sender_endpoint.port();
			LOCATOR_ADDRESS_INVALID(m_senderLocator.address);
			if(m_sender_endpoint.address().is_v4())
			{
				for(int i=0;i<4;i++)
				{
					m_senderLocator.address[i+12] = m_sender_endpoint.address().to_v4().to_bytes()[i];
				}
			}
			else
			{
				for(int i=0;i<16;i++)
				{
					m_senderLocator.address[i] = m_sender_endpoint.address().to_v6().to_bytes()[i];
				}
			}
		}
		catch(boost::system::system_error const& e)
		{
			logError(RTPS_MSG_IN,"Boost error: " << e.what());
            lock.lock();
            runningAsync_ = false;
            lock.unlock();
            cond_.notify_one();
			this->putToListen();
			return;
		}
        
		try
		{
			mp_listenResource->mp_receiver->processCDRMsg(mp_RTPSParticipantImpl->getGuid().guidPrefix,
					&m_senderLocator,
					&mp_listenResource->mp_receiver->m_rec_msg);
		}
		catch(int e)
		{
			logError(RTPS_MSG_IN,IDSTRING"Error processing message: " << e,C_BLUE);

		}

		logInfo(RTPS_MSG_IN,IDSTRING " Message of size "<< mp_listenResource->mp_receiver->m_rec_msg.length <<" processed" ,C_BLUE);
        lock.lock();
        runningAsync_ = false;
        lock.unlock();
        cond_.notify_one();
		this->putToListen();
	}
	else if(err == boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_MSG_IN,IDSTRING"Operation in listening socket aborted...",C_BLUE);
		return;
	}
	else
	{
		//CDRMessage_t msg;
		logInfo(RTPS_MSG_IN,IDSTRING"Msg processed, Socket async receive put again to listen ",C_BLUE);
		this->putToListen();
	}
}

// NOTE This version allways listen in ANY.
void ListenResourceImpl::getLocatorAddresses(Locator_t& loc, bool isMulti)
{
	const char* const METHOD_NAME = "getLocatorAddresses";

    logInfo(RTPS_MSG_IN,"Defined Locator IP with 0s (listen to all interfaces), listening to all interfaces",C_BLUE);
    LocatorList_t myIP;

    if(loc.kind == LOCATOR_KIND_UDPv4)
    {
        IPFinder::getIP4Address(&myIP);
        m_listen_endpoint.address(boost::asio::ip::address_v4());
    }
    else if(loc.kind == LOCATOR_KIND_UDPv6)
    {
        IPFinder::getIP6Address(&myIP);
        m_listen_endpoint.address(boost::asio::ip::address_v6());
    }

    if(!isMulti)
    {
        for(auto lit = myIP.begin();lit!= myIP.end();++lit)
        {
            lit->port = loc.port;
            mv_listenLoc.push_back(*lit);
        }
    }
    else
    {
        mv_listenLoc.push_back(loc);
    }

	m_listen_endpoint.port((uint16_t)loc.port);
}


bool ListenResourceImpl::init_thread(RTPSParticipantImpl* pimpl,Locator_t& loc, uint32_t listenSocketSize, bool isMulti, bool isFixed)
{
	const char* const METHOD_NAME = "init_thread";
	this->mp_RTPSParticipantImpl = pimpl;
	if(loc.kind == LOCATOR_KIND_INVALID)
		return false;

	getLocatorAddresses(loc, isMulti);
	logInfo(RTPS_MSG_IN,"Initializing in : "<<mv_listenLoc,C_BLUE);
	boost::asio::ip::address multiaddress;
	//OPEN THE SOCKET:
	m_listen_socket.open(m_listen_endpoint.protocol());
	m_listen_socket.set_option(boost::asio::socket_base::receive_buffer_size(listenSocketSize));
	if(isMulti)
	{
		m_listen_socket.set_option( boost::asio::ip::udp::socket::reuse_address( true ) );
		m_listen_socket.set_option( boost::asio::ip::multicast::enable_loopback( true ) );

        if(loc.kind == LOCATOR_KIND_UDPv4)
        {
            multiaddress = boost::asio::ip::address_v4::from_string(loc.to_IP4_string().c_str());
        }
        else if(loc.kind == LOCATOR_KIND_UDPv6)
        {
			boost::asio::ip::address_v6::bytes_type bt;
			for (uint8_t i = 0; i < 16; ++i)
				bt[i] = loc.address[i];
            multiaddress = boost::asio::ip::address_v6(bt);
        }
	}

	if(isFixed)
	{
		try
		{
			m_listen_socket.bind(m_listen_endpoint);
		}
		catch (boost::system::system_error const& e)
		{
			logError(RTPS_MSG_IN,"Error: " << e.what() << " : " << m_listen_endpoint,C_BLUE);
			return false;
		}
	}
	else
	{
		bool binded = false;
		for(uint8_t i = 0; i < 100; ++i) //TODO make it configurable by user.
		{
			m_listen_endpoint.port(m_listen_endpoint.port()+i);
			try
			{
				m_listen_socket.bind(m_listen_endpoint);
				binded = true;
				break;
			}
			catch(boost::system::system_error const& )
			{
				logInfo(RTPS_MSG_IN,"Tried port "<< m_listen_endpoint.port() << ", trying next...",C_BLUE);
			}
		}

		if(!binded)
		{
			logError(RTPS_MSG_IN,"Tried 100 ports and none was working, last tried: "<< m_listen_endpoint,C_BLUE);
			return false;
		}
		else
		{
			for(auto lit = mv_listenLoc.begin();lit!=mv_listenLoc.end();++lit)
				lit->port = m_listen_endpoint.port();
		}
	}

	boost::asio::socket_base::receive_buffer_size option;
	m_listen_socket.get_option(option);
	logInfo(RTPS_MSG_IN,"Created: " << m_listen_endpoint<< " || Listen buffer size: " << option.value(),C_BLUE);
	if(isMulti)
	{
		joinMulticastGroup(multiaddress);
	}
	this->putToListen();
	mp_thread = new boost::thread(&ListenResourceImpl::run_io_service,this);
	mp_RTPSParticipantImpl->ResourceSemaphoreWait();
	return true;
}

void ListenResourceImpl::joinMulticastGroup(boost::asio::ip::address& addr)
{
	const char* const METHOD_NAME = "joinMulticastGroups";
	logInfo(RTPS_MSG_IN,"Joining group: "<<mv_listenLoc,C_BLUE);
	try
	{
		LocatorList_t loclist;
		if(m_listen_endpoint.address().is_v4())
		{
			IPFinder::getIP4Address(&loclist);
			for(LocatorListIterator it=loclist.begin();it!=loclist.end();++it)
				m_listen_socket.set_option( boost::asio::ip::multicast::join_group(addr.to_v4(),
						boost::asio::ip::address_v4::from_string(it->to_IP4_string())) );
		}
		else if(m_listen_endpoint.address().is_v6())
		{
			IPFinder::getIP6Address(&loclist);
			int index = 0;
			for(LocatorListIterator it=loclist.begin();it!=loclist.end();++it)
			{
				//			boost::asio::ip::address_v6::bytes_type bt;
				//			for (uint8_t i = 0; i < 16;++i)
				//				bt[i] = it->address[i];
				m_listen_socket.set_option(
						boost::asio::ip::multicast::join_group(
								addr.to_v6(),index
						));
				++index;
			}
		}
	}
	catch(boost::system::system_error const& e)
	{
		logError(RTPS_MSG_IN,"Boost error: "<< e.what());
	}
}

void ListenResourceImpl::putToListen()
{
	const char* const METHOD_NAME = "putToListen";
	CDRMessage::initCDRMsg(&mp_listenResource->mp_receiver->m_rec_msg);
	try
	{
		m_listen_socket.async_receive_from(
				boost::asio::buffer((void*)mp_listenResource->mp_receiver->m_rec_msg.buffer,
						mp_listenResource->mp_receiver->m_rec_msg.max_size),
						m_sender_endpoint,
						boost::bind(&ListenResourceImpl::newCDRMessage, this,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));
	}
	catch(boost::system::system_error const& e)
	{
		logError(RTPS_MSG_IN,"Boost error: "<< e.what());
	}
}

void ListenResourceImpl::run_io_service()
{
	const char* const METHOD_NAME = "run_io_service";
	try
	{
		logInfo(RTPS_MSG_IN,"Thread: " << boost::this_thread::get_id() << " listening in IP: " << m_listen_socket.local_endpoint(),C_BLUE) ;

		mp_RTPSParticipantImpl->ResourceSemaphorePost();

		this->m_io_service.run();
	}
	catch(boost::system::system_error const& e)
	{
		logError(RTPS_MSG_IN,"Boost error: "<<e.what());
	}
}
}
} /* namespace rtps */
} /* namespace eprosima */
