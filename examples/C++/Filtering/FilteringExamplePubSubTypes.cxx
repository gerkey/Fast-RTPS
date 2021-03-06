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

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "FilteringExamplePubSubTypes.h"

FilteringExamplePubSubType::FilteringExamplePubSubType() {
	setName("FilteringExample");
	m_typeSize = (uint32_t)FilteringExample::getMaxCdrSerializedSize();
	m_isGetKeyDefined = FilteringExample::isKeyDefined();
	m_keyBuffer = (unsigned char*)malloc(FilteringExample::getKeyMaxCdrSerializedSize()>16 ? FilteringExample::getKeyMaxCdrSerializedSize() : 16);
}

FilteringExamplePubSubType::~FilteringExamplePubSubType() {
	if(m_keyBuffer!=nullptr)
		free(m_keyBuffer);
}

bool FilteringExamplePubSubType::serialize(void *data, SerializedPayload_t *payload) {
	FilteringExample *p_type = (FilteringExample*) data;
	eprosima::fastcdr::FastBuffer fastbuffer((char*) payload->data, payload->max_size); // Object that manages the raw buffer.
	eprosima::fastcdr::Cdr ser(fastbuffer); 	// Object that serializes the data.
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
	p_type->serialize(ser); 	// Serialize the object:
    payload->length = (uint16_t)ser.getSerializedDataLength(); 	//Get the serialized length
	return true;
}

bool FilteringExamplePubSubType::deserialize(SerializedPayload_t* payload, void* data) {
	FilteringExample* p_type = (FilteringExample*) data; 	//Convert DATA to pointer of your type
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); 	// Object that manages the raw buffer.
	eprosima::fastcdr::Cdr deser(fastbuffer, payload->encapsulation == CDR_BE ? eprosima::fastcdr::Cdr::BIG_ENDIANNESS : eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS); 	// Object that deserializes the data.
	p_type->deserialize(deser);	//Deserialize the object:
	return true;
}

void* FilteringExamplePubSubType::createData() {
	return (void*)new FilteringExample();
}

void FilteringExamplePubSubType::deleteData(void* data) {
	delete((FilteringExample*)data);
}

bool FilteringExamplePubSubType::getKey(void *data, InstanceHandle_t* handle) {
	if(!m_isGetKeyDefined)
		return false;
	FilteringExample* p_type = (FilteringExample*) data;
	eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer,FilteringExample::getKeyMaxCdrSerializedSize()); 	// Object that manages the raw buffer.
	eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS); 	// Object that serializes the data.
	p_type->serializeKey(ser);
	if(FilteringExample::getKeyMaxCdrSerializedSize()>16)	{
		m_md5.init();
		m_md5.update(m_keyBuffer,(unsigned int)ser.getSerializedDataLength());
		m_md5.finalize();
		for(uint8_t i = 0;i<16;++i)    	{
        	handle->value[i] = m_md5.digest[i];
    	}
    }
    else    {
    	for(uint8_t i = 0;i<16;++i)    	{
        	handle->value[i] = m_keyBuffer[i];
    	}
    }
	return true;
}

