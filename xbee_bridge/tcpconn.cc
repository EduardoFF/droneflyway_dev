#include "tcpconn.h"
#include <glog/logging.h>
#define IT(c) __typeof((c).begin())
#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)

using namespace std;



TCPConn::TCPConn(string  ip,
		 int port,
		 bool autorun):
  m_acceptor(NULL),
  m_stream(NULL),
  m_ip(ip),
  m_port(port)
{
  pthread_mutex_init(&m_mutex, NULL);
  initSocket();
  if( autorun )
    run();
  
}


void
TCPConn::initSocket()
{
  m_acceptor = new TCPAcceptor(m_port, m_ip.c_str());  
}

bool
TCPConn::run()
{
    int status = pthread_create(&m_thread, NULL, internalThreadEntryFunc, this);
    m_running=true;
    return (status == 0);
}


/// returns time in milliseconds
uint64_t
TCPConn::getTime()
{
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);

    uint64_t ms1 = (uint64_t) timestamp.tv_sec;
    ms1*=1000;

    uint64_t ms2 = (uint64_t) timestamp.tv_usec;
    ms2/=1000;

    return (ms1+ms2);
}

std::string
TCPConn::getTimeStr()
{
#ifndef FOOTBOT_LQL_SIM
    char buffer [80];
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    char currentTime[84] = "";
    sprintf(currentTime, "%s:%d", buffer, milli);
    std::string ctime_str(currentTime);
    return ctime_str;
#else
    return "mytime";
#endif
}

void
TCPConn::internalThreadEntry()
{

  if (m_acceptor->start() == 0)
    {
      while (!m_abort)
	{
	  m_stream = m_acceptor->accept();
	  if (m_stream != NULL)
	    {
	      ssize_t len;
	      char line[125];
	      while ((len = m_stream->receive(line, sizeof(line))) > 0)
		{
		  line[len] = 0;
		  LOG(INFO) << "TCP Received " << len;
		  //printf("received - %s\n", line);
		  if( m_receiveCB )
		    {
		      m_receiveCB(line,len);
		    }
		}
	      delete m_stream;
	    }
	}
    }
}

void
TCPConn::registerReceive(TCPConn::ReceiveCB cb)
{
  m_receiveCB = cb;
}

void
TCPConn::send(const char *data, size_t len)
{
  if( m_stream != NULL )
    {
      pthread_mutex_lock(&m_mutex);
      m_stream->send(data, len);
      pthread_mutex_unlock(&m_mutex);
    }
}

TCPConn::~TCPConn()
{
  if( isRunning() )
    {
      stop();
      pthread_join(m_thread,NULL);
    }
  if( m_acceptor )
    delete m_acceptor;
  if( m_stream )
    delete m_stream;

}
