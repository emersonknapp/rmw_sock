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
#include <string>
#include <vector>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"


using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Util::ServerApplication;
using Poco::Util::Application;


/// Handle a WebSocket connection.
class WebSocketRequestHandler : public HTTPRequestHandler
{
public:
  void handleRequest(HTTPServerRequest & request, HTTPServerResponse & response)
  {
    Application & app = Application::instance();
    try {
      WebSocket ws(request, response);
      app.logger().information("WebSocket connection established.");
      char buffer[1024];
      int flags;
      int n;
      do {
        n = ws.receiveFrame(buffer, sizeof(buffer), flags);
        buffer[n] = '\0';
        printf("Frame received (length=%d): %s\n", n, buffer);
        ws.sendFrame(buffer, n, flags);
      } while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
      app.logger().information("WebSocket connection closed.");
    } catch (WebSocketException & exc) {
      app.logger().log(exc);
      switch (exc.code()) {
        case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
          response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
        // fallthrough
        case WebSocket::WS_ERR_NO_HANDSHAKE:
        case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
        case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
          response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
          response.setContentLength(0);
          response.send();
          break;
      }
    }
  }
};


class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
  HTTPRequestHandler * createRequestHandler(const HTTPServerRequest & request)
  {
    if (false) {
      logRequest(request);
    }
    return new WebSocketRequestHandler;
  }

private:
  void logRequest(const HTTPServerRequest & request)
  {
    Application & app = Application::instance();
    app.logger().information("Request from " +
      request.clientAddress().toString() + ": " +
      request.getMethod() + " " +
      request.getURI() + " " +
      request.getVersion());

    for (const auto it : request) {
      app.logger().information(it.first + ": " + it.second);
    }
  }
};


class WebSocketServer : public Poco::Util::ServerApplication
{
public:
  WebSocketServer() = default;
  virtual ~WebSocketServer() = default;

protected:
  int main(const std::vector<std::string> & /* args */) override
  {
    const unsigned short port = 8080;
    ServerSocket svs(port);
    HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
    srv.start();
    printf("Server running on port %u...\n", port);
    waitForTerminationRequest();
    srv.stop();
    return Application::EXIT_OK;
  }
};

POCO_SERVER_MAIN(WebSocketServer)
