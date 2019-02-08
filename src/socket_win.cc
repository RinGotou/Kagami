#include "module.h"

namespace kagami {
  Message NewTCPClient(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["port"]), "Invalid port string.");
    CONDITION_ASSERT(IsStringObject(p["addr"]), "Invalid address string.");
    CONDITION_ASSERT(IsStringObject(p["buf_size"]), "Invalid buffer size.");

    string port = ParseRawString(p["port"].Cast<string>());
    string addr = ParseRawString(p["addr"].Cast<string>());
    size_t buf_size = stol(p["buf_size"].Cast<string>());

    shared_ptr<TCPClient> client_ptr(
      make_shared<TCPClient>(port, addr, buf_size)
    );

    return Message().SetObject(
      Object(client_ptr, kTypeIdTCPClient)
    );
  }

  Message TCPClientStart(ObjectMap &p) {
    auto &client = p[kStrObject].Cast<TCPClient>();
    return Message(util::MakeBoolean(client.StartClient()));
  }

  Message TCPConnectorSend(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["content"]), "Invalid content string.");
    auto connector = std::dynamic_pointer_cast<TCPConnector>(p[kStrObject].Get());
    string content = ParseRawString(p["content"].Cast<string>());
    return Message(util::MakeBoolean(connector->Send(content)));
  }

  Message TCPConnectorReceive(ObjectMap &p) {
    auto &dest = p["dest"];
    auto connector = std::dynamic_pointer_cast<TCPConnector>(p[kStrObject].Get());
    string dest_buf;
    string result = util::MakeBoolean(connector->Receive(dest_buf));
    dest.ManageContent(make_shared<string>(dest_buf), kTypeIdString);
    return Message(result);
  }

  Message TCPConnectorClose(ObjectMap &p) {
    auto connector = std::dynamic_pointer_cast<TCPConnector>(p[kStrObject].Get());
    connector->Close();
    return Message();
  }

  Message TCPConnectorGood(ObjectMap &p) {
    auto connector = std::dynamic_pointer_cast<TCPConnector>(p[kStrObject].Get());
    return Message(util::MakeBoolean(connector->Good()));
  }

  Message WSockInfoResultCode(ObjectMap &p) {
    auto info = std::dynamic_pointer_cast<WSockInfo>(p[kStrObject].Get());
    return Message(to_string(info->GetLastResultCode()));
  }

  Message GetWSALastError(ObjectMap &p) {
    return Message(to_string(WSAGetLastError()));
  }

  Message NewTCPServer(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["port"]), "Invalid port string.");
    CONDITION_ASSERT(IsStringObject(p["buf_size"]), "Invalid buffer size.");
    string port = ParseRawString(p["port"].Cast<string>());
    size_t buf_size = stol(p["buf_size"].Cast<string>());

    shared_ptr<TCPServer> server_ptr(
      make_shared<TCPServer>(port, buf_size)
    );

    return Message().SetObject(
      Object(server_ptr, kTypeIdTCPServer)
    );
  }

  Message TCPServerStart(ObjectMap &p) {
    auto &server = p[kStrObject].Cast<TCPServer>();
    auto &backlog_obj = p["backlog"];
    int backlog = SOMAXCONN;

    if (!backlog_obj.Null()) {
      backlog = stoi(
        ParseRawString(backlog_obj.Cast<string>())
      );
    }

    return Message(util::MakeBoolean(server.StartServer(backlog)));
  }

  Message TCPServerAccept(ObjectMap &p) {
    auto &server = p[kStrObject].Cast<TCPServer>();
    shared_ptr<TCPServer::ClientConnector> connector_ptr(
      make_shared<TCPServer::ClientConnector>(server.Accept())
    );
    return Message().SetObject(
      Object(connector_ptr, kTypeIdClientConnector)
    );
  }

  Message TCPServerClose(ObjectMap &p) {
    auto &server = p[kStrObject].Cast<TCPServer>();
    server.Close();
    return Message();
  }

  void LoadSocketStuff() {
    using management::type::NewTypeSetup;
    using management::CreateInterface;
    
    CreateInterface(Interface(GetWSALastError, "", "WSALastError"));

    NewTypeSetup(kTypeIdTCPClient, SimpleSharedPtrCopy<TCPClient>)
      .InitConstructor(
        Interface(NewTCPClient, "port|addr|buf_size", "TCPClient")
      )
      .InitMethods(
        {
          Interface(TCPClientStart, "", "start"),
          Interface(TCPConnectorSend, "content", "send"),
          Interface(TCPConnectorReceive, "dest", "receive"),
          Interface(TCPConnectorGood, "", "good"),
          Interface(TCPConnectorClose, "", "close"),
          Interface(WSockInfoResultCode, "", "result_code")
        }
    );

    NewTypeSetup(kTypeIdTCPServer, SimpleSharedPtrCopy<TCPServer>)
      .InitConstructor(
        Interface(NewTCPServer, "port|buf_size", "TCPServer")
      )
      .InitMethods(
        {
          Interface(TCPServerStart, "backlog", "start"),
          Interface(TCPServerAccept, "", "accept"),
          Interface(TCPServerClose, "", "close"),
          Interface(WSockInfoResultCode, "", "result_code")
        }
    );

    NewTypeSetup(kTypeIdClientConnector, SimpleSharedPtrCopy<TCPServer::ClientConnector>)
      .InitMethods(
        {
          Interface(TCPConnectorSend, "content", "send"),
          Interface(TCPConnectorReceive, "dest", "receive"),
          Interface(TCPConnectorGood, "", "good"),
          Interface(TCPConnectorClose, "", "close"),
          Interface(WSockInfoResultCode, "", "result_code")
        }
    );
  }
}