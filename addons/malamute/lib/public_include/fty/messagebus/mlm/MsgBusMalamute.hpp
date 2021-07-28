/*  =========================================================================
    fty_common_messagebus_malamute - class description

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#pragma once

#include "fty/messagebus/IMessageBus.hpp"
#include "fty/messagebus/MsgBusException.hpp"
#include "fty/messagebus/mlm/MsgBusMlmMessage.hpp"
#include "fty/messagebus/utils/MsgBusHelper.hpp"

#include <fty_common_mlm.h>

namespace fty::messagebus::mlm
{
  // Default malamute end point
  static auto constexpr DEFAULT_MLM_END_POINT{"ipc://@/malamute"};

  using MessageListener = fty::messagebus::MessageListener<MlmMessage>;

  typedef void(MalamuteMessageListenerFn)(const char*, const char*, zmsg_t**);
  using MalamuteMessageListener = std::function<MalamuteMessageListenerFn>;

  class MessageBusMalamute final : public IMessageBus<MlmMessage>
  {
  public:
    MessageBusMalamute() = delete;
    MessageBusMalamute(const std::string& clientName, const std::string& endpoint);

    ~MessageBusMalamute() override;

    fty::messagebus::ComState connect() override;

    // Async topic
    DeliveryState publish(const std::string& topic, const MlmMessage& message) override;
    DeliveryState subscribe(const std::string& topic, MessageListener messageListener) override;
    DeliveryState unsubscribe(const std::string& topic, MessageListener messageListener) override;

    // Async queue
    DeliveryState sendRequest(const std::string& requestQueue, const MlmMessage& message) override;
    DeliveryState sendRequest(const std::string& requestQueue, const MlmMessage& message, MessageListener messageListener) override;
    DeliveryState sendReply(const std::string& replyQueue, const MlmMessage& message) override;
    DeliveryState receive(const std::string& queue, MessageListener messageListener) override;

    // Sync queue
    Opt<MlmMessage> request(const std::string& requestQueue, const MlmMessage& message, int receiveTimeOut) override;

  private:
    static void listener(zsock_t* pipe, void* ptr);
    void listenerMainloop(zsock_t* pipe);
    void listenerHandleMailbox(const char*, const char*, zmsg_t*);
    void listenerHandleStream(const char*, const char*, zmsg_t*);

    mlm_client_t* m_client;
    std::string m_clientName;
    std::string m_endpoint;
    std::string m_publishTopic;

    zactor_t* m_actor;
    std::map<std::string, MessageListener> m_subscriptions;

    std::condition_variable m_cv;
    std::mutex m_cv_mtx;
    MlmMessage m_syncResponse;
    std::string m_syncUuid;
  };
} // namespace fty::messagebus::mlm

void fty_common_messagebus_malamute_test(bool verbose);
