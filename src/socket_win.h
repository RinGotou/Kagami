#include "machine.h"

#if defined (_WIN32)
namespace kagami {
  template <class Tx>
  Message TCPConnectorSend(ObjectMap &p) {
    EXPECT_TYPE(p, "content", kTypeIdString);
    Tx &tx = p[kStrMe].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    string content = p["content"].Cast<string>();
    return Message().SetObject(connector->Send(content));
  }

  template<class Tx>
  Message TCPConnectorReceive(ObjectMap &p) {
    auto &dest = p["dest"];
    Tx &tx = p[kStrMe].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    string dest_buf;
    bool result = connector->Receive(dest_buf);
    dest.PackContent(make_shared<string>(dest_buf), kTypeIdString);
    return Message().SetObject(result);
  }

  template <class Tx>
  Message TCPConnectorClose(ObjectMap &p) {
    Tx &tx = p[kStrMe].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    connector->Close();
    return Message();
  }

  template <class Tx>
  Message TCPConnectorGood(ObjectMap &p) {
    Tx &tx = p[kStrMe].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    return Message().SetObject(connector->Good());
  }

  template <class Tx>
  Message WSockInfoResultCode(ObjectMap &p) {
    Tx &tx = p[kStrMe].Cast<Tx>();
    WSockInfo *info = dynamic_cast<WSockInfo *>(&tx);
    return Message().SetObject(
      static_cast<int64_t>(info->GetLastResultCode())
    );
  }
}
#endif
