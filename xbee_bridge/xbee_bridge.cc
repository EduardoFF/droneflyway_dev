#include <iostream>
#include <stdlib.h>
#include "xbee_interface.h"
#include <glog/logging.h>
#include "tcpconn.h"
using namespace std;

#ifndef IT
#define IT(c) __typeof((c).begin())
#endif

#ifndef FOREACH
#define FOREACH(i,c) for(__typeof((c).begin()) i=(c).begin();i!=(c).end();++i)
#endif


XbeeInterface *g_xbee;
char g_outBuf[130];
bool g_abort;

/// Mutex used to send one packet at a time
pthread_mutex_t g_sendMutex;
int g_nPacketsSent;
std::map<uint8_t, int> g_nPacketsRcv;
std::map<uint8_t, uint16_t>  g_lastSeqnRcv;

using namespace std;


TCPConn *g_tcpConn;

void
signalHandler( int signum )
{
    g_abort = true;
    printf("ending app...\n");
    LOG(INFO) << "Ending app";
    /// stop timers

    delete g_xbee;
    delete g_tcpConn;
    LOG(INFO) << "Clean exit";

    exit(signum);
}



void xbeeSend(uint8_t dst, size_t buflen)
{
  if( buflen > 100 )
    {
      fprintf(stderr, "ERROR: buflen: %d > 100 \n", buflen);
      return;
      //      exit(1)
    }
    
  pthread_mutex_lock(&g_sendMutex);
  XbeeInterface::TxInfo txInfo;
  txInfo.reqAck = true;
  txInfo.readCCA = false;
  
  int retval = g_xbee->send(dst, txInfo, g_outBuf, buflen);
  if( retval == XbeeInterface::NO_ACK )
    {
      DLOG(INFO) << "send failed NOACK";
    }
  else if( retval == XbeeInterface::TX_MAC_BUSY )
    {
      DLOG(INFO) << "send failed MACBUSY";
    }
  else
    {
      DLOG(INFO) << "send OK";
      g_nPacketsSent++;
    }
  pthread_mutex_unlock(&g_sendMutex);
}

/// Function that receives the messages and handles them per type
void
receiveData(uint16_t addr,
	    void *data,
	    char rssi, timespec timestamp, size_t len)
{
  //   printf("got msg of len %u\n", len);
    g_tcpConn->send(static_cast<const char*>(data), len);
}
 

void
receiveTCP(void *data, size_t len)
{
  DLOG(INFO) << "Got TCP: len: " << len;
  memcpy(g_outBuf, data, len);
  DLOG(INFO) << "Forwarding to Xbee";
  xbeeSend(1, len);
}

/*--------------------------------------------------------------------
 * main()
 * Main function to set up ROS node.
 *------------------------------------------------------------------*/
int main(int argc, char **argv)
{

  /// register signal
  signal(SIGINT, signalHandler);

  /// Initialize Log
  google::InitGoogleLogging(argv[0]);
  FLAGS_logbufsecs = 0;
  DLOG(INFO) << "Logging initialized";

  string  xbeeDev = argv[1];
  string  xbeeMode = "xbee1";
  int baudrate = 57600;
  uint8_t nodeId = 0;
  
  g_abort = false;
  g_nPacketsSent=0;
    
  /// Xbee PARAMETERS
  XbeeInterfaceParam xbeePar;
  xbeePar.SourceAddress = nodeId;
  xbeePar.brate = baudrate;
  xbeePar.mode  = xbeeMode;
  xbeePar.Device = xbeeDev;
  xbeePar.writeParams = false;

  /// create mutexes
  if (pthread_mutex_init(&g_sendMutex, NULL) != 0)
    {
      fprintf(stderr, "mutex init failed\n");
      fflush(stderr);
      exit(-1);
    }

    
  g_xbee = new XbeeInterface(xbeePar);
  if( g_xbee->isOK() )
    {
      /// Listen for messages
      g_xbee->registerReceive(&receiveData);
    }
  else
    {
      fprintf(stderr, "xbee init failed\n");
      exit(-1);
    }
  

  g_tcpConn = new TCPConn("127.0.0.1", 12345, true);
  g_tcpConn->registerReceive(&receiveTCP);
			

  
  // Main loop.
  for(;;)
  {
    // Publish the message.
    //    node_example->publishMessage(&pub_message);

    if( !g_xbee->checkAlive() )
      printf("Xbee is not alive!!\n");
    sleep(5);
      
  }

  return 0;
} // end main()