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

class IOBluetoothMemoryDescriptorRetainer;
class IOBluetoothSCOMemoryDescriptorRetainer;

extern void BluetoothSleepTimeOutOccurred(OSObject *,IOTimerEventSource *);

#define ulong unsigned long
#define uchar unsigned char

class IOBluetoothHostControllerTransport : public IOService
{
    OSDeclareAbstractStructors(IOBluetoothHostControllerTransport)
    
public:

    virtual bool init(OSDictionary *){return true;};
    virtual void free(void){};
    virtual IOService * probe(IOService *,int *){return NULL;};
    virtual bool start(IOService *){return false;};
    virtual void stop(IOService *){};
    virtual bool terminate(uint){return false;};
    virtual int terminateAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual bool terminateWL(uint) {return false;};
    virtual bool InitializeTransport(void) {return false;};
    virtual int InitializeTransportAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn InitializeTransportWL(IOService *) {return 1;};
    virtual bool setTransportWorkLoop(void *,IOWorkLoop *) {return 0;};
    virtual IOWorkLoop * getWorkLoop() const {return NULL;};
    virtual IOCommandGate *getCommandGate(void) {return NULL;};
    virtual int getPropertyFromTransport(OSSymbol const*) {return 0;};
    virtual int getPropertyFromTransport(OSString const*) {return 0;};
    virtual int getPropertyFromTransport(char const*) {return 0;};
    virtual IOReturn CallConfigPM(void) {return 0;};
    virtual unsigned long maxCapabilityForDomainState(ulong) {return 0;};
    virtual bool ConfigurePM(IOService *) {return false;};
    virtual unsigned long initialPowerStateForDomainState(ulong) {return 0;};
    virtual IOReturn RequestTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState,char *) {return 1;};
    virtual IOReturn WaitForControllerPowerState(IOBluetoothHCIControllerInternalPowerState,char *) {return 0;};
    virtual IOReturn WaitForControllerPowerStateWithTimeout(IOBluetoothHCIControllerInternalPowerState,uint,char *,bool){return 0;};
    virtual void CompletePowerStateChange(char *) {};
    virtual IOReturn ProcessPowerStateChangeAfterResumed(char *) {return 1;};
    virtual IOReturn setAggressiveness(ulong,ulong){return 0;};
    virtual IOReturn setAggressivenessAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn setAggressivenessWL(ulong,ulong){return 0;};
    virtual IOReturn setPowerState(ulong,IOService *){return 0;};
    virtual IOReturn setPowerStateAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn setPowerStateWL(ulong,IOService *) {return 0;};
    virtual IOReturn powerStateWillChangeTo(ulong,ulong,IOService *){return 0;};
    virtual IOReturn powerStateWillChangeToAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn powerStateWillChangeToWL(uint,void *){return 0;};
    virtual void systemWillShutdown(uint){};
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
    virtual IOReturn TransportSendSCOData(void *) {return 0;};
    virtual IOReturn SendHCIRequest(uchar *,ulong long) {return 0;};
    virtual IOReturn UpdateSCOConnections(uchar,uint) {return 0;};
    virtual IOReturn ToggleLMPLogging(uchar *) {return 0;};
    virtual IOReturn TransportLMPLoggingBulkOutWrite(uchar,uchar) {return 0;};
    virtual IOBluetoothHCIControllerInternalPowerState GetCurrentPowerState(void){return kIO80211SystemPowerStateAwake;};
    virtual IOBluetoothHCIControllerInternalPowerState GetPendingPowerState(void){return kIO80211SystemPowerStateAwake;};
    virtual IOReturn ChangeTransportPowerStateFromIdleToOn(char *){return 0;};
    virtual IOReturn ChangeTransportPowerState(ulong,bool,IOBluetoothHCIControllerInternalPowerState,char *){return 0;};
    virtual IOReturn WaitForControllerPowerStateChange(IOBluetoothHCIControllerInternalPowerState,char *){return 0;};
    virtual IOReturn WakeupSleepingPowerStateThread(void){return 0;};
    virtual bool ControllerSupportWoBT(void) {return 0;};
    virtual int GetControllerVendorID(void) {return 0;};
    virtual int GetControllerProductID(void) {return 0;};
    virtual int GetRadioPowerState(void) {return 0;};
    void SetRadioPowerState(uchar) {};
    virtual int GetNVRAMSettingForSwitchBehavior(void){return 0;};
    virtual int GetControllerLocationID(void){return 0;};
    virtual bool GetBuiltIn(void){return 0;};
    virtual bool GetSupportPowerOff(void){return 0;};
    virtual bool IsHardwareInitialized(void){return 0;};
    virtual int GetHardwareStatus(void){return 0;};
    virtual void ResetHardwareStatus(void){};
    virtual UInt32 ConvertAddressToUInt32(void *){return 0;};
    virtual bool SetActiveController(bool){return 0;};
    virtual IOReturn ResetBluetoothDevice(void) {return 0;};
    virtual IOReturn TransportCommandSleep(void *,uint,char *,bool){return 0;};
    virtual IOReturn ReadyToGo(bool){return 0;};
    virtual bool TerminateCalled(void){return 0;};
    virtual int GetInfo(void *) {return 0;};
    virtual IOReturn CallPowerManagerChangePowerStateTo(ulong,char *){return 0;};
    virtual int GetControllerTransportType(void){return 0;};
    virtual int SupportNewIdlePolicy(void){return 0;};
    virtual int CheckACPIMethodsAvailabilities(void){return 0;};
    virtual IOReturn SetBTRS(void){return 0;};
    virtual IOReturn SetBTPU(void){return 0;};
    virtual IOReturn SetBTPD(void){return 0;};
    virtual IOReturn SetBTRB(bool){return 0;};
    virtual IOReturn SetBTLP(bool){return 0;};
    virtual void NewSCOConnection(void) {};
    virtual void retain(void) {};
    virtual void release(void) {};
    virtual IOReturn RetainTransport(char *){return 0;};
    virtual IOReturn ReleaseTransport(char *){return 0;};
    virtual IOReturn SetIdlePolicyValue(uint) {return 0;};
    virtual bool TransportWillReEnumerate(void) {return 0;};
    virtual void ConvertPowerFlagsToString(ulong,char *){};
    virtual void SetupTransportSCOParameters(void) {};
    virtual void DestroyTransportSCOParameters(void) {};
    virtual IOReturn WaitForSystemReadyForSleep(char *){return 0;};
    virtual IOReturn StartBluetoothSleepTimer(void){return 0;};
    virtual IOReturn CancelBluetoothSleepTimer(void){return 0;};
    virtual void *CreateOSLogObject(void){return 0;};
    virtual IOReturn setProperties(OSObject *){return 0;};
    virtual IOReturn setPropertiesAction(OSObject *,void *,void *,void *,void *){return 0;};
    virtual IOReturn setPropertiesWL(OSObject *){return 0;};
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
