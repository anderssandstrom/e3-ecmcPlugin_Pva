/*************************************************************************\
* Copyright (c) 2019 European Spallation Source ERIC
* ecmc is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
*
*  ecmcPv.h
*
*  Created on: May 12, 2020
*      Author: anderssandstrom
*
\*************************************************************************/

#ifndef ECMC_PV_H_
#define ECMC_PV_H_

#include "ecmcPvDefs.h"
#include <atomic>  
#include <iostream>
#include <pv/pvaClient.h>

#include "epicsThread.h"
#include "epicsMutex.h"

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

enum ecmc_pva_cmd {
  ECMC_PV_CMD_NONE = 0,  
  ECMC_PV_CMD_PUT  = 2
};

 class ecmcPv;
 typedef std::tr1::shared_ptr<ecmcPv> ecmcPvPtr;

class ecmcPv :  public PvaClientChannelStateChangeRequester,
                public PvaClientMonitorRequester,
                public PvaClientPutRequester,
                public std::tr1::enable_shared_from_this<ecmcPv>
{
 public:
  POINTER_DEFINITIONS(ecmcPv);
  ecmcPv(const std::string &channelName,
         const std::string &providerName,
         const std::string &request, 
         int index);
  ecmcPv();

  ~ecmcPv();

  static ecmcPvPtr create(PvaClientPtr const & pvaClient,
                          const std::string  & channelName, 
                          const std::string  & providerName,
                          const std::string  & request,
                          int index);
  void   init(PvaClientPtr const &pvaClient);
  PvaClientMonitorPtr getPvaClientMonitor();
  int    getError();
  int    reset();
  void   start(const string &request);
  void   stop();  
  void   putCmd(double value); // Async Commads
  double getLastReadValue();
  bool   busy();
  bool   connected();
  void   exeCmdThread();
  std::string getChannelName();
  std::string getProviderName();
  virtual void monitorConnect(epics::pvData::Status const & status,
                              PvaClientMonitorPtr const & monitor,
                              epics::pvData::StructureConstPtr const & structure);
  virtual void channelPutConnect (const epics::pvData::Status &status,
                                  PvaClientPutPtr const &clientPut);
  virtual void event(PvaClientMonitorPtr const & monitor);
  virtual void channelStateChange(PvaClientChannelPtr const & channel,
                                  bool isConnected);
  virtual void putDone(const epics::pvData::Status & status,
                       PvaClientPutPtr const & clientPut);

 private:
  int    validateType(PvaClientMonitorDataPtr monData);
  double getDouble(PvaClientMonitorDataPtr monData);
  void   putDouble(double value);
  static std::string to_string(int value);

  std::string  channelName_;
  std::string  providerName_;
  std::string  request_;
  bool         channelConnected_;
  bool         monitorConnected_;
  bool         putConnected_;
  bool         isStarted_;
  bool         typeValidated_;
  bool         destructs_;
  int          index_;
  int          errorCode_;  
  double       valueLatestRead_;
  double       valueToWrite_;  
  Type         type_;  
  ecmc_pva_cmd cmd_;
  std::atomic_flag busyLock_;  
  
  // General
  PvaClientPtr        pva_;
  PvaClientChannelPtr pvaClientChannel_;    
  
  // Put
  PvaClientPutPtr     pvaClientPut_;

  // Monitor       
  PvaClientMonitorPtr pvaClientMonitor_;
  
  // Thread related
  epicsEvent          doCmdEvent_;
  epicsThreadId       cmdExeThread_;
  epicsMutexId        ecmcGetValMutex_;  
};

#endif  /* ECMC_PV_H_ */
