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

/*
 * AckNackMsg.hpp
 *
 */


namespace eprosima{
namespace fastrtps{
namespace rtps{

bool RTPSMessageCreator::addMessageAcknack(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidPrefix,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag){
	const char* const METHOD_NAME = "addMessageAcknack";
	try
	{
		RTPSMessageCreator::addHeader(msg,guidprefix);
        RTPSMessageCreator::addSubmessageInfoDST(msg, remoteGuidPrefix);
		RTPSMessageCreator::addSubmessageAcknack(msg,readerId, writerId,SNSet,count,finalFlag);
		msg->length = msg->pos;
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Data message not created"<<e<<endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageAcknack(CDRMessage_t* msg,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag)
{
	const char* const METHOD_NAME = "addSubmessageAcknack";
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
	CDRMessage::initCDRMsg(&submsgElem);
	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
    submsgElem.msg_endian = BIGEND;
#else
	flags = flags | BIT(0);
	submsgElem.msg_endian  = LITTLEEND;
#endif

	if(finalFlag)
		flags = flags | BIT(1);
	
	try{
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumberSet(&submsgElem,&SNSet);
		CDRMessage::addInt32(&submsgElem,count);
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Message creator fails"<<e<<endl)
		return false;
	}

	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg,ACKNACK, flags, (uint16_t)submsgElem.length);
	//Append Submessage elements to msg

	CDRMessage::appendMsg(msg, &submsgElem);

	g_pool_submsg.release_CDRMsg(submsgElem);
	msg->length = msg->pos;
	return true;
}

bool RTPSMessageCreator::addMessageNackFrag(CDRMessage_t* msg, const GuidPrefix_t& guidprefix,
	const GuidPrefix_t& remoteGuidPrefix,
	const EntityId_t& readerId, const EntityId_t& writerId,
	SequenceNumber_t& writerSN, FragmentNumberSet_t fnState, int32_t count){
	const char* const METHOD_NAME = "addMessageNackFrag";
	try
	{
		RTPSMessageCreator::addHeader(msg, guidprefix);
		RTPSMessageCreator::addSubmessageInfoDST(msg, remoteGuidPrefix);
		RTPSMessageCreator::addSubmessageNackFrag(msg, readerId, writerId, writerSN, fnState, count);
		msg->length = msg->pos;
	}
	catch (int e)
	{
		logError(RTPS_CDR_MSG, "Data message not created" << e << endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageNackFrag(CDRMessage_t* msg,
	const EntityId_t& readerId, const EntityId_t& writerId,
	SequenceNumber_t& writerSN, FragmentNumberSet_t fnState, int32_t count)
{
	const char* const METHOD_NAME = "addSubmessageNackFrag";
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
	CDRMessage::initCDRMsg(&submsgElem);
	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
	submsgElem.msg_endian = BIGEND;
#else
	flags = flags | BIT(0);
	submsgElem.msg_endian = LITTLEEND;
#endif

	try{
		CDRMessage::addEntityId(&submsgElem, &readerId);
		CDRMessage::addEntityId(&submsgElem, &writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem, &writerSN);
		// Add fragment number status
		CDRMessage::addFragmentNumberSet(&submsgElem, &fnState);
		CDRMessage::addUInt32(&submsgElem, count);
	}
	catch (int e)
	{
		logError(RTPS_CDR_MSG, "Message creator fails" << e << endl)
			return false;
	}

	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg, NACK_FRAG, flags, (uint16_t)submsgElem.length);
	//Append Submessage elements to msg

	CDRMessage::appendMsg(msg, &submsgElem);

	g_pool_submsg.release_CDRMsg(submsgElem);
	msg->length = msg->pos;
	return true;
}

}
}
}




