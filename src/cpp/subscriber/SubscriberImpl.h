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
 * @file SubscriberImpl.h
 *
 */

#ifndef SUBSCRIBERIMPL_H_
#define SUBSCRIBERIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastrtps/rtps/common/Locator.h>
#include <fastrtps/rtps/common/Guid.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastrtps/rtps/reader/ReaderListener.h>


namespace eprosima {
namespace fastrtps {
namespace rtps
{
class RTPSReader;
class RTPSParticipant;
}

using namespace rtps;

class TopicDataType;
class SubscriberListener;
class ParticipantImpl;
class SampleInfo_t;
class Subscriber;

/**
 * Class SubscriberImpl, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTRTPS_MODULE
 */
class SubscriberImpl {
	friend class ParticipantImpl;
public:

	/**
	* @param p
	* @param ptype
	* @param attr
	* @param listen
	*/
	SubscriberImpl(ParticipantImpl* p,TopicDataType* ptype,
			SubscriberAttributes& attr,SubscriberListener* listen = nullptr);
	virtual ~SubscriberImpl();

	/**
	 * Method to block the current thread until an unread message is available
	 */
	void waitForUnreadMessage();


	/** @name Read or take data methods.
	 * Methods to read or take data from the History.
	 */

	///@{

	bool readNextData(void* data,SampleInfo_t* info);
	bool takeNextData(void* data,SampleInfo_t* info);

	///@}
	
	/**
	 * Update the Attributes of the subscriber;
	 * @param att Reference to a SubscriberAttributes object to update the parameters;
	 * @return True if correctly updated, false if ANY of the updated parameters cannot be updated
	 */
	bool updateAttributes(SubscriberAttributes& att);

	/**
	* Get associated GUID
	* @return Associated GUID
	*/
	const GUID_t& getGuid();

	/**
	 * Get the Attributes of the Subscriber.
	 * @return Attributes of the Subscriber.
	 */
	SubscriberAttributes getAttributes(){return m_att;}

	/**
	* Get topic data type
	* @return Topic data type
	*/
	TopicDataType* getType() {return mp_type;};

private:
	//!Participant
	ParticipantImpl* mp_participant;

	//!Pointer to associated RTPSReader
	RTPSReader* mp_reader;
	//! Pointer to the TopicDataType object.
	TopicDataType* mp_type;
	//!Attributes of the Subscriber
	SubscriberAttributes m_att;
	//!History
	SubscriberHistory m_history;
	//!Listener
	SubscriberListener* mp_listener;
	class SubscriberReaderListener : public ReaderListener
	{
	public:
		SubscriberReaderListener(SubscriberImpl* s): mp_subscriberImpl(s){};
		virtual ~SubscriberReaderListener(){};
		void onReaderMatched(RTPSReader* reader,MatchingInfo& info);
		void onNewCacheChangeAdded(RTPSReader * reader,const CacheChange_t* const change);
		SubscriberImpl* mp_subscriberImpl;
	}m_readerListener;

	Subscriber* mp_userSubscriber;
	//!RTPSParticipant
		RTPSParticipant* mp_rtpsParticipant;
};


} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* SUBSCRIBERIMPL_H_ */
