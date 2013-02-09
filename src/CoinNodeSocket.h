////////////////////////////////////////////////////////////////////////////////
//
// CoinNodeSocket.h
//
// Copyright (c) 2011 Eric Lombrozo
//

#ifndef COINNODESOCKET_H__
#define COINNODESOCKET_H__

#include "CoinNodeData.h"

#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <iostream>

#include <boost/thread/mutex.hpp>

typedef struct hostent SHostent;
typedef struct sockaddr_in SockaddrIn;

namespace Coin {

class CoinNodeSocket;
class CoinNodeAbstractListener;

typedef void (*CoinMessageHandler)(CoinNodeSocket* pNodeSocket, const Coin::CoinNodeMessage& message);
typedef void (*SocketClosedHandler)(CoinNodeSocket* pNodeSocket, int code);

class CoinNodeSocket
{
private:
    int h_socket;
    SHostent* p_host;
    SockaddrIn serverAddress;
    uint32_t m_magic;
    uchar_vector m_magicBytes;
    uint32_t m_version;
    std::string m_hostname;
    uint m_port;

    CoinMessageHandler coinMessageHandler;
    pthread_t h_messageThread;
    pthread_mutex_t m_sendLock;
    
    SocketClosedHandler socketClosedHandler;
    
    bool m_multithreaded;

public:
    void* pAppData;
    CoinNodeAbstractListener* pListener;

    pthread_mutex_t m_handshakeLock;
    pthread_cond_t m_handshakeComplete;
    pthread_mutex_t m_updateAppDataLock;

    boost::mutex open_close_mutex;

    pthread_t h_lastCallbackThread;

    CoinNodeSocket();
    ~CoinNodeSocket() { this->close(); } // TODO: wait on messageHandler threads.

    void setMultithreaded(bool m_multithreaded) { this->m_multithreaded = m_multithreaded; }
    bool isMultithreaded() const { return this->m_multithreaded; }

    int getSocketHandle() const { return h_socket; }
    uchar_vector getMagicBytes() const { return m_magicBytes; }
    uint32_t getMagic() const { return m_magic; }
    uint getVersion() const { return m_version; }
    std::string getHostname() const { return m_hostname; }
    uint getPort() const { return m_port; }
    CoinMessageHandler getMessageHandler() const { return coinMessageHandler; }
    SocketClosedHandler getSocketClosedHandler() const { return socketClosedHandler; }

    void open(CoinMessageHandler callback, uint32_t magic, uint32_t version, const char* hostname = "127.0.0.1", uint port = 8333,
              SocketClosedHandler socketClosedHandler = NULL);
    void close();

    void doHandshake(
        int32_t version,
        uint64_t services,
        int64_t timestamp,
        const Coin::NetworkAddress& recipientAddress,
        const Coin::NetworkAddress& senderAddress,
        uint64_t nonce,
        const char* subVersion,
        int32_t startHeight
    );

    void waitOnHandshakeComplete();

    void sendMessage(unsigned char* command, const std::vector<unsigned char>& payload);
    void sendMessage(const Coin::CoinNodeMessage& pMessage);
};

}; // namespace Coin
#endif