//
//  IOBluetoothHostControllerTransport.h
//  IntelBluetoothFirmware
//
//  Created by qcwap on 2019/11/19.
//  Copyright © 2019 zxystd. All rights reserved.
//

#ifndef IOBluetoothHostControllerTransport_h
#define IOBluetoothHostControllerTransport_h

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

enum IOBluetoothHCIControllerInternalPowerState
{
    kIO80211SystemPowerStateUnknown,
    kIO80211SystemPowerStateAwake,
    kIO80211SystemPowerStateSleeping,
};

typedef enum IOBluetoothHCIControllerInternalPowerState IOBluetoothHCIControllerInternalPowerState;

class IOBluetoothMemoryDescriptorRetainer;
class IOBluetoothSCOMemoryDescriptorRetainer;

extern void BluetoothSleepTimeOutOccurred(OSObject *,IOTimerEventSource *);

#define ulong unsigned long
#define uchar unsigned char

class IOBluetoothHostControllerTransport : public IOService
{
    OSDeclareAbstractStructors(IOBluetoothHostControllerTransport)
    
public:
    virtual bool init(OSDictionary *);
    virtual void free(void);
    virtual IOService * probe(IOService *,int *);
    virtual bool start(IOService *);
    virtual void stop(IOService *);
    virtual bool terminate(uint);
    virtual int terminateAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual bool terminateWL(uint) {return false;};
    virtual bool InitializeTransport(void);
    virtual int InitializeTransportAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn InitializeTransportWL(IOService *) {return 1;};
    virtual bool setTransportWorkLoop(void *,IOWorkLoop *);
    virtual IOWorkLoop * getWorkLoop() const {return NULL;};
    virtual IOCommandGate *getCommandGate(void) {return NULL;};
    virtual int getPropertyFromTransport(OSSymbol const*);
    virtual int getPropertyFromTransport(OSString const*);
    virtual int getPropertyFromTransport(char const*);
    virtual IOReturn CallConfigPM(void);
    virtual unsigned long maxCapabilityForDomainState(ulong) {return 0;};
    virtual bool ConfigurePM(IOService *) {return false;};
    virtual unsigned long initialPowerStateForDomainState(ulong) {return 0;};
    virtual IOReturn RequestTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState,char *) {return 1;};
    virtual IOReturn WaitForControllerPowerState(IOBluetoothHCIControllerInternalPowerState,char *);
    virtual IOReturn WaitForControllerPowerStateWithTimeout(IOBluetoothHCIControllerInternalPowerState,uint,char *,bool);
    virtual void CompletePowerStateChange(char *) {};
    virtual IOReturn ProcessPowerStateChangeAfterResumed(char *) {return 1;};
    virtual IOReturn setAggressiveness(ulong,ulong);
    virtual IOReturn setAggressivenessAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn setAggressivenessWL(ulong,ulong);
    virtual IOReturn setPowerState(ulong,IOService *);
    virtual IOReturn setPowerStateAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn setPowerStateWL(ulong,IOService *) {return 0;};
    virtual IOReturn powerStateWillChangeTo(ulong,ulong,IOService *){return 0;};
    virtual IOReturn powerStateWillChangeToAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn powerStateWillChangeToWL(uint,void *){return 0;};
    virtual void systemWillShutdown(uint);
    virtual IOReturn systemWillShutdownAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn systemWillShutdownWL(uint,void *) {return 0;};
    virtual IOReturn SetRemoteWakeUp(bool) {return 0;};
    virtual IOReturn DoDeviceReset(ushort) {return 0;};
    virtual void AbortPipesAndClose(bool,bool) {};
    virtual bool HostSupportsSleepOnUSB(void) {return 1;};
    virtual IOReturn StartInterruptPipeRead(void) {return 1;};
    virtual bool StopInterruptPipeRead(void) {return 1;};
    virtual bool StartLMPLogging(void) {return 1;};
    virtual bool StartLMPLoggingBulkPipeRead(void) {return 1;};
    virtual IOReturn StartBulkPipeRead(void) {return 1;};
    virtual bool StopBulkPipeRead(void) {return 1;};
    virtual IOReturn TransportBulkOutWrite(void *) {return 0;};
    virtual IOReturn TransportIsochOutWrite(void *,void *,int) {return 0;};
    virtual IOReturn TransportSendSCOData(void *);
    virtual IOReturn SendHCIRequest(uchar *,ulong long) {return 0;};
    virtual IOReturn UpdateSCOConnections(uchar,uint) {return 0;};
    virtual IOReturn ToggleLMPLogging(uchar *) {return 0;};
    virtual IOReturn TransportLMPLoggingBulkOutWrite(uchar,uchar) {return 0;};
    virtual IOBluetoothHCIControllerInternalPowerState GetCurrentPowerState(void);
    virtual IOBluetoothHCIControllerInternalPowerState GetPendingPowerState(void);
    virtual IOReturn ChangeTransportPowerStateFromIdleToOn(char *);
    virtual IOReturn ChangeTransportPowerState(ulong,bool,IOBluetoothHCIControllerInternalPowerState,char *);
    virtual IOReturn WaitForControllerPowerStateChange(IOBluetoothHCIControllerInternalPowerState,char *);
    virtual IOReturn WakeupSleepingPowerStateThread(void);
    virtual bool ControllerSupportWoBT(void) {return 0;};
    virtual int GetControllerVendorID(void) {return 0;};
    virtual int GetControllerProductID(void) {return 0;};
    virtual int GetRadioPowerState(void) {return 0;};
    virtual void SetRadioPowerState(uchar) {};
    virtual int GetNVRAMSettingForSwitchBehavior(void);
    virtual int GetControllerLocationID(void);
    virtual bool GetBuiltIn(void){return 0;};
    virtual bool GetSupportPowerOff(void);
    virtual bool IsHardwareInitialized(void);
    virtual int GetHardwareStatus(void);
    virtual void ResetHardwareStatus(void);
    virtual UInt32 ConvertAddressToUInt32(void *);
    virtual bool SetActiveController(bool);
    virtual IOReturn ResetBluetoothDevice(void) {return 0;};
    virtual IOReturn TransportCommandSleep(void *,uint,char *,bool);
    virtual IOReturn ReadyToGo(bool);
    virtual bool TerminateCalled(void);
    virtual int GetInfo(void *) {return 0;};
    virtual IOReturn CallPowerManagerChangePowerStateTo(ulong,char *);
    virtual int GetControllerTransportType(void);
    virtual int SupportNewIdlePolicy(void);
    virtual int CheckACPIMethodsAvailabilities(void);
    virtual IOReturn SetBTRS(void);
    virtual IOReturn SetBTPU(void);
    virtual IOReturn SetBTPD(void);
    virtual IOReturn SetBTRB(bool);
    virtual IOReturn SetBTLP(bool);
    virtual void NewSCOConnection(void);
    virtual void retain(void) {};
    virtual void release(void) {};
    virtual IOReturn RetainTransport(char *);
    virtual IOReturn ReleaseTransport(char *);
    virtual IOReturn SetIdlePolicyValue(uint) {return 0;};
    virtual bool TransportWillReEnumerate(void) {return 0;};
    virtual void ConvertPowerFlagsToString(ulong,char *);
    virtual void SetupTransportSCOParameters(void) {};
    virtual void DestroyTransportSCOParameters(void) {};
    virtual IOReturn WaitForSystemReadyForSleep(char *);
    virtual IOReturn StartBluetoothSleepTimer(void);
    virtual IOReturn CancelBluetoothSleepTimer(void);
    virtual void *CreateOSLogObject(void);
    virtual IOReturn setProperties(OSObject *);
    virtual IOReturn setPropertiesAction(OSObject *,void *,void *,void *,void *);
    virtual IOReturn setPropertiesWL(OSObject *);
    virtual IOReturn HardReset(void) {return 3758097084LL;};
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  0);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  1);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  2);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  3);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  4);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  5);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  6);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  7);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  8);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport,  9);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 10);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 11);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 12);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 13);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 14);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 15);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 16);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 17);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 18);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 19);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 20);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 21);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 22);
    OSMetaClassDeclareReservedUnused( IOBluetoothHostControllerTransport, 23);
};

#endif /* IOBluetoothHostControllerTransport_h */
