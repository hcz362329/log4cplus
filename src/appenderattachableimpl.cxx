// Module:  Log4CPLUS
// File:    appenderattachableimpl.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
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


#include <log4cplus/appender.h>
#include <log4cplus/helpers/appenderattachableimpl.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/thread/syncprims-pub-impl.h>

#include <algorithm>


namespace log4cplus
{


namespace spi
{


AppenderAttachable::~AppenderAttachable() = default;


} // namespace spi


namespace helpers
{


//////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::AppenderAttachableImpl ctor and dtor
//////////////////////////////////////////////////////////////////////////////

AppenderAttachableImpl::AppenderAttachableImpl() = default;


AppenderAttachableImpl::~AppenderAttachableImpl() = default;



///////////////////////////////////////////////////////////////////////////////
// log4cplus::helpers::AppenderAttachableImpl public methods
///////////////////////////////////////////////////////////////////////////////

void
AppenderAttachableImpl::addAppender(SharedAppenderPtr newAppender)
{
    if (! newAppender)
    {
        getLogLog().warn( LOG4CPLUS_TEXT("Tried to add NULL appender") );
        return;
    }

    std::unique_lock guard {appender_list_mutex};

    auto it = std::find(appenderList.begin(), appenderList.end(), newAppender);
    if (it == appenderList.end())
    {
        appenderList.push_back(newAppender);
    }
}



AppenderAttachableImpl::ListType
AppenderAttachableImpl::getAllAppenders()
{
    std::unique_lock guard {appender_list_mutex};

    return appenderList;
}



SharedAppenderPtr
AppenderAttachableImpl::getAppender(const log4cplus::tstring& name)
{
    std::unique_lock guard {appender_list_mutex};

    for (SharedAppenderPtr & ptr : appenderList)
    {
        if (ptr->getName() == name)
            return ptr;
    }

    return SharedAppenderPtr ();
}



void
AppenderAttachableImpl::removeAllAppenders()
{
    std::unique_lock guard {appender_list_mutex};

    // Clear appenders in specific order because the order of destruction of
    // std::vector elements is surprisingly unspecified and it breaks our
    // tests' expectations.

    for (auto & app : appenderList)
        app = SharedAppenderPtr ();

    appenderList.clear ();
}



void
AppenderAttachableImpl::removeAppender(SharedAppenderPtr appender)
{
    if (! appender)
    {
        getLogLog().warn( LOG4CPLUS_TEXT("Tried to remove NULL appender") );
        return;
    }

    std::unique_lock guard {appender_list_mutex};

    auto it = std::find(appenderList.begin(), appenderList.end(), appender);
    if (it != appenderList.end())
    {
        appenderList.erase(it);
    }
}



void
AppenderAttachableImpl::removeAppender(const log4cplus::tstring& name)
{
    removeAppender(getAppender(name));
}



int
AppenderAttachableImpl::appendLoopOnAppenders(const spi::InternalLoggingEvent& event) const
{
    int count = 0;

    std::unique_lock guard {appender_list_mutex};

    for (auto & appender : appenderList)
    {
        ++count;
        appender->doAppend(event);
    }

    return count;
}


} // namespace helpers


} // namespace log4cplus
