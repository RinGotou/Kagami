#include "socket_win.h"

#if defined (_WIN32)
namespace kagami {
  Message NewTCPClient(ObjectMap &p) {
    EXPECT_TYPE(p, "port", kTypeIdString);
    EXPECT_TYPE(p, "addr", kTypeIdString);
    EXPECT_TYPE(p, "buf_size", kTypeIdInt);

    string port = p["port"].Cast<string>();
    string addr = p["addr"].Cast<string>();
    size_t buf_size = p["buf_size"].Cast<int64_t>();

    shared_ptr<TCPClient> client_ptr(
      new TCPClient(port, addr, buf_size)
    );

    return Message().SetObject(
      Object(client_ptr, kTypeIdTCPClient)
    );
  }

  Message TCPClientStart(ObjectMap &p) {
    auto &client = p[kStrMe].Cast<TCPClient>();
    bool result = client.StartClient();
    return Message().SetObject(result);
  }

  Message GetWSALastError(ObjectMap &p) {
    return Message().SetObject(
      Object(make_shared<int64_t>(WSAGetLastError()), kTypeIdInt)
    );
  }

  Message NewTCPServer(ObjectMap &p) {
    EXPECT_TYPE(p, "port", kTypeIdString);
    EXPECT_TYPE(p, "buf_size", kTypeIdInt);

    string port = p["port"].Cast<string>();
    size_t buf_size = p["buf_size"].Cast<int64_t>();

    shared_ptr<TCPServer> server_ptr(
      new TCPServer(port, buf_size)
    );

    return Message().SetObject(
      Object(server_ptr, kTypeIdTCPServer)
    );
  }

  Message TCPServerStart(ObjectMap &p) {
    auto &server = p[kStrMe].Cast<TCPServer>();
    auto &backlog_obj = p["backlog"];
    int backlog = SOMAXCONN;
    auto str = backlog_obj.Cast<string>();

    if (!backlog_obj.Null()) {
      from_chars(str.data(), str.data() + str.size(), backlog);
    }

    bool result = server.StartServer();

    return Message().SetObject(result);
  }

  Message TCPServerAccept(ObjectMap &p) {
    auto &server = p[kStrMe].Cast<TCPServer>();
    shared_ptr<TCPServer::ClientConnector> connector_ptr(
      new TCPServer::ClientConnector(server.Accept())
    );
    return Message().SetObject(
      Object(connector_ptr, kTypeIdClientConnector)
    );
  }

  Message TCPServerClose(ObjectMap &p) {
    auto &server = p[kStrMe].Cast<TCPServer>();
    server.Close();
    return Message();
  }

  Message WinSockStartup(ObjectMap &p) {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    return Message().SetObject(result == 0);
  }

  Message WinSockCleanup(ObjectMap &p) {
    WSACleanup();
    return Message();
  }

  void LoadSocketStuff() {
    using management::type::NewTypeSetup;
    using management::CreateNewInterface;
    using management::type::PointerHasher;
    using ClientConnector = TCPServer::ClientConnector;
    
    CreateNewInterface(Interface(GetWSALastError, "", "WSALastError"));
    CreateNewInterface(Interface(WinSockStartup, "", "WSAStartup"));
    CreateNewInterface(Interface(WinSockCleanup, "", "WSACleanup"));

    NewTypeSetup(kTypeIdTCPClient, FakeCopy<TCPClient>, PointerHasher())
      .InitConstructor(
        Interface(NewTCPClient, "port|addr|buf_size", "TCPClient")
      )
      .InitMethods(
        {
          Interface(TCPClientStart, "", "start"),
          Interface(TCPConnectorSend<TCPClient>, "content", "send"),
          Interface(TCPConnectorReceive<TCPClient>, "dest", "receive"),
          Interface(TCPConnectorGood<TCPClient>, "", "good"),
          Interface(TCPConnectorClose<TCPClient>, "", "close"),
          Interface(WSockInfoResultCode<TCPClient>, "", "result_code")
        }
    );

    NewTypeSetup(kTypeIdTCPServer, FakeCopy<TCPServer>, PointerHasher())
      .InitConstructor(
        Interface(NewTCPServer, "port|buf_size", "TCPServer")
      )
      .InitMethods(
        {
          Interface(TCPServerStart, "backlog", "start", kCodeAutoFill),
          Interface(TCPServerAccept, "", "accept"),
          Interface(TCPServerClose, "", "close"),
          Interface(WSockInfoResultCode<TCPServer>, "", "result_code")
        }
    );

    NewTypeSetup(kTypeIdClientConnector, FakeCopy<ClientConnector>, PointerHasher())
      .InitMethods(
        {
          Interface(TCPConnectorSend<ClientConnector>, "content", "send"),
          Interface(TCPConnectorReceive<ClientConnector>, "dest", "receive"),
          Interface(TCPConnectorGood<ClientConnector>, "", "good"),
          Interface(TCPConnectorClose<ClientConnector>, "", "close"),
          Interface(WSockInfoResultCode<ClientConnector>, "", "result_code")
        }
    );
  }
}
#endif
