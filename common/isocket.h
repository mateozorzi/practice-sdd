#ifndef ISOCKET_H
#define ISOCKET_H


class ISocket {


public:
    virtual ~ISocket() = default;
    virtual int sendsome(const void* data, unsigned int sz) = 0;
    virtual int recvsome(void* data, unsigned int sz) = 0;
    virtual int sendall(const void* data, unsigned int sz) = 0;
    virtual int recvall(void* data, unsigned int sz) = 0;
    virtual void shutdown(int how) = 0;
    virtual bool is_stream_send_closed() const = 0;
    virtual bool is_stream_recv_closed() const = 0;
    virtual int close() = 0;
};


#endif
