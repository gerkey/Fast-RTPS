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

/*************************************************************************
 * @file Data1mb.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace { char dummy; }
#endif

#include "Data1mb.h"

#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

Data1mb::Data1mb()
{
}

Data1mb::~Data1mb()
{
}

Data1mb::Data1mb(const Data1mb &x)
{
    m_data = x.m_data;
}

Data1mb::Data1mb(Data1mb &&x)
{
    m_data = std::move(x.m_data);
}

Data1mb& Data1mb::operator=(const Data1mb &x)
{
    m_data = x.m_data;
    
    return *this;
}

Data1mb& Data1mb::operator=(Data1mb &&x)
{
    m_data = std::move(x.m_data);
    
    return *this;
}

bool Data1mb::operator==(const Data1mb &x) const
{
    if(m_data == x.m_data)
        return true;

    return false;
}

size_t Data1mb::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;
            
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += (1024000 * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);


    return current_alignment - initial_alignment;
}

void Data1mb::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    if(m_data.size() <= 1024000)
    scdr << m_data;
    else
        throw eprosima::fastcdr::exception::BadParamException("data field exceeds the maximum length");
}

void Data1mb::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_data;
}

size_t Data1mb::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
	size_t current_align = current_alignment;
            

    return current_align;
}

bool Data1mb::isKeyDefined()
{
    return false;
}

void Data1mb::serializeKey(eprosima::fastcdr::Cdr& /*scdr*/) const
{
}
