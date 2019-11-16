// Copyright 2019 Emerson Knapp
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
#include <iostream>

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;


int main()
{
  HTTPClientSession cs("localhost", 8080);
  HTTPRequest request(HTTPRequest::HTTP_GET, "/?encoding=text", HTTPMessage::HTTP_1_1);
  request.set("origin", "localhost");
  HTTPResponse response;

  try {
    WebSocket * m_psock = new WebSocket(cs, request, response);
    char const * testStr = "Hello echo websocket!";
    char receiveBuff[256];

    int len = m_psock->sendFrame(testStr, strlen(testStr), WebSocket::FRAME_TEXT);
    std::cout << "Sent bytes " << len << std::endl;
    int flags = 0;

    int rlen = m_psock->receiveFrame(receiveBuff, 256, flags);
    std::cout << "Received bytes " << rlen << std::endl;
    std::cout << receiveBuff << std::endl;

    m_psock->close();
  } catch (std::exception & e) {
    std::cout << "Exception " << e.what();
  }
}
