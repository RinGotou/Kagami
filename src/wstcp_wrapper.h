#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <memory>
#pragma comment(lib, "ws2_32.lib")

namespace suzu {
  using std::shared_ptr;

  class WSockInfo {
  protected:
    int result_code_;

  public:
    virtual ~WSockInfo() {}

    int GetLastResultCode() const {
      return result_code_;
    }

    int LastError() const {
      return WSAGetLastError();
    }
  };

  class AddrInfo {
  public:
    addrinfo *ptr_;

  public:
    virtual ~AddrInfo() { freeaddrinfo(ptr_); }

    AddrInfo() : ptr_(nullptr) {}

    AddrInfo(addrinfo *ptr) : ptr_(ptr) {}

    void ManageContent(addrinfo *ptr) { ptr_ = ptr; }
    addrinfo *Get() { return ptr_; }
  };

  using AddrContent = shared_ptr<AddrInfo>;

  class WSockTCPFamily : public WSockInfo {
  protected:
    std::string port_;
    std::string addr_;

    AddrContent addr_content_;

    size_t buf_size_;

    SOCKET connector_;

  public:
    virtual ~WSockTCPFamily() {}

    WSockTCPFamily() = delete;

    WSockTCPFamily(const WSockTCPFamily &rhs) :
      port_(rhs.port_), addr_(rhs.addr_), addr_content_(rhs.addr_content_), 
      buf_size_(rhs.buf_size_), connector_(INVALID_SOCKET) {}

    WSockTCPFamily(const WSockTCPFamily &&rhs) :
      WSockTCPFamily(rhs) {}

    WSockTCPFamily(std::string port, std::string addr, size_t buf_size) :
      port_(port), addr_(addr), addr_content_(new AddrInfo()),
      buf_size_(buf_size), connector_(INVALID_SOCKET) {}

  protected:
     bool InitSocket(int flags = 0, int addr_family = AF_UNSPEC) {
      addrinfo hints;
      addrinfo *addr_info_;
      ZeroMemory(&hints, sizeof(hints));

      if (flags != 0) {
        hints.ai_flags = flags;
      }
      hints.ai_family = addr_family;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;

      result_code_ = getaddrinfo(
        addr_.empty()? nullptr : addr_.c_str(),
        port_.c_str(),
        &hints,
        &addr_info_
      );

      if (result_code_ == 0) {
        connector_ = socket(
          addr_info_->ai_family,
          addr_info_->ai_socktype,
          addr_info_->ai_protocol
        );

        addr_content_->ManageContent(addr_info_);
      }

      return (result_code_ == 0 && connector_ != INVALID_SOCKET);
    }

  public:
    void Close() {
      closesocket(connector_);
    }
  };

  class TCPConnector {
  public:
    virtual bool Send(std::string) = 0;
    virtual bool Receive(std::string &) = 0;
    virtual void Close() = 0;
    virtual bool Good() const = 0;
    virtual ~TCPConnector() {}
  };

  class TCPClient : 
    public WSockTCPFamily , 
    public TCPConnector {  
  public:
    TCPClient() = delete;

    TCPClient(std::string port, std::string addr, size_t buf_size) :
      WSockTCPFamily(port, addr, buf_size) {}

    TCPClient(const TCPClient &rhs) :
      WSockTCPFamily(rhs) {}

    TCPClient(const TCPClient &&rhs) :
      TCPClient(rhs) {}

    //Connect
    bool StartClient() {
      if (!InitSocket()) return false;

      addrinfo *node = addr_content_->Get();

      while (node != nullptr) {
        result_code_ = connect(
          connector_,
          node->ai_addr,
          static_cast<int>(node->ai_addrlen)
        );

        if (result_code_ == SOCKET_ERROR) {
          node = node->ai_next;
          continue;
        }
        else {
          break;
        }
      }

      return (result_code_ != SOCKET_ERROR);
    }

    bool Send(std::string content) override {
      if (content.size() + 1 > buf_size_) {
        return false;
      }

      result_code_ = send(
        connector_,
        content.c_str(),
        static_cast<int>(content.size()),
        0
      );

      return (result_code_ != SOCKET_ERROR);
    }

    bool Receive(std::string &dest) override {
      char *recv_buf = new char[buf_size_];
      ZeroMemory(recv_buf, buf_size_);

      result_code_ = recv(
        connector_,
        recv_buf,
        static_cast<int>(buf_size_),
        0
      );

      if (result_code_ > 0) {
        dest = std::string(recv_buf);
      }

      return (result_code_ >= 0);
    }

    //Repeat Close() function because of TCPConnector interface
    void Close() {
      closesocket(connector_);
    }

    bool Good() const {
      return (connector_ != INVALID_SOCKET);
    }
  };

  class TCPServer : public WSockTCPFamily {
  public:
    class ClientConnector : 
      public TCPConnector,
      public WSockInfo {
    protected:
      SOCKET connector_;
      size_t buf_size_;

    public:
      ClientConnector() = delete;

      ClientConnector(SOCKET socket, size_t buf_size) : 
        connector_(socket), buf_size_(buf_size) {}

      bool Send(std::string content) override {
        if (content.size() + 1 > buf_size_) {
          return false;
        }
        
        result_code_ = send(
          connector_,
          content.c_str(),
          static_cast<int>(buf_size_),
          0
        );

        return (result_code_ != SOCKET_ERROR);
      }

      bool Receive(std::string &dest) override {
        char *recv_buf = new char[buf_size_];
        ZeroMemory(recv_buf, buf_size_);

        result_code_ = recv(
          connector_,
          recv_buf,
          static_cast<int>(buf_size_),
          0
        );

        if (result_code_ > 0) {
          dest = std::string(recv_buf);
        }

        return (result_code_ >= 0);
      }

      void Close() {
        closesocket(connector_);
      }

      bool Good() const {
        return (connector_ != INVALID_SOCKET);
      }
    };

  public:
    ~TCPServer() {
      closesocket(connector_);
    }

    TCPServer(std::string port, size_t buf_size) :
      WSockTCPFamily(port, std::string(), buf_size) {}

    TCPServer(const TCPServer &rhs) :
      WSockTCPFamily(rhs) {}

    TCPServer(const TCPServer &&rhs) :
      TCPServer(rhs) {}

    //Binding and Listening
    bool StartServer(int backlog = SOMAXCONN) {
      if (!InitSocket()) return false;

      addrinfo *node = addr_content_->Get();
      
      while (node != nullptr) {
        result_code_ = bind(
          connector_,
          node->ai_addr,
          static_cast<int>(node->ai_addrlen)
        );

        if (result_code_ == SOCKET_ERROR) {
          node = node->ai_next;
          continue;
        }
        else {
          break;
        }
      }

      if (result_code_ != SOCKET_ERROR) {
        result_code_ = listen(
          connector_,
          backlog
        );
      }

      return (result_code_ != SOCKET_ERROR);
    }

    ClientConnector Accept() {
      SOCKET client_connection = accept(connector_, nullptr, nullptr);
      return ClientConnector(client_connection, buf_size_);
    }
  };
}
