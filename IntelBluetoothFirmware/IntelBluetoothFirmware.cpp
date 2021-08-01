//
//  IntelBluetoothFirmware.cpp
//  IntelBluetoothFirmware
//
//  Created by zxystd on 2019/11/17.
//  Copyright © 2019 zxystd. All rights reserved.
//

#include "BtIntel.h"

#include "IntelBluetoothFirmware.hpp"
#include <libkern/libkern.h>
#include <libkern/OSKextLib.h>
#include <libkern/version.h>
#include <libkern/OSTypes.h>
#include <IOKit/usb/StandardUSB.h>
#include "Hci.h"
#include "linux.h"
#include "Log.h"

#include "IntelBluetoothOpsGen1.hpp"
#include "IntelBluetoothOpsGen2.hpp"
#include "IntelBluetoothOpsGen3.hpp"

#define super IOService
OSDefineMetaClassAndStructors(IntelBluetoothFirmware, IOService)

//com.apple.iokit.IOBluetoothHostControllerUSBTransport

enum { kMyOffPowerState = 0, kMyOnPowerState = 1 };

#define kIOPMPowerOff 0

static IOPMPowerState myTwoStates[2] =
{
    {1, kIOPMPowerOff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

bool IntelBluetoothFirmware::init(OSDictionary *dictionary)
{
    XYLog("Driver init()\n");
    return super::init(dictionary);
}

void IntelBluetoothFirmware::free() {
    XYLog("Driver free()\n");
    super::free();
}

bool IntelBluetoothFirmware::start(IOService *provider)
{
    XYLog("Driver Start()\n");
    char fwName[64];
    m_pDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (m_pDevice == NULL) {
        XYLog("Driver Start fail, not usb device\n");
        return false;
    }
    PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
    makeUsable();
    
    if (!super::start(provider)) {
        return false;
    }
    
    if (!m_pDevice->open(this)) {
        XYLog("start fail, can not open device\n");
        cleanUp();
        stop(this);
        return false;
    }
    switch (currentType) {
        case kTypeGen1:
            m_pBTIntel = new IntelBluetoothOpsGen1();
            break;
        case kTypeGen2:
            m_pBTIntel = new IntelBluetoothOpsGen2();
            break;
        case kTypeGen3:
            m_pBTIntel = new IntelBluetoothOpsGen3();
            break;
    }
    if (!m_pBTIntel->initWithDevice(this, m_pDevice)) {
        XYLog("start fail, can not init device\n");
        cleanUp();
        stop(this);
        return false;
    }
    XYLog("BT init succeed\n");
    if (!m_pBTIntel->setup()) {
        cleanUp();
        stop(this);
        return false;
    }
    m_pBTIntel->getFirmwareName(fwName, sizeof(fwName));
    publishReg(true, fwName);
    cleanUp();
    return true;
}

void IntelBluetoothFirmware::publishReg(bool isSucceed, const char *fwName)
{
    m_pDevice->setProperty("FirmwareLoaded", isSucceed);
    if (isSucceed) {
        setProperty("fw_name", OSString::withCString(fwName));
    }
}

void IntelBluetoothFirmware::cleanUp()
{
    XYLog("Clean up...\n");
    OSSafeReleaseNULL(m_pBTIntel);
    if (m_pDevice) {
        if (m_pDevice->isOpen(this)) {
            m_pDevice->close(this);
        }
        m_pDevice = NULL;
    }
}

IOReturn IntelBluetoothFirmware::setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice)
{
//    XYLog("setPowerState powerStateOrdinal=%lu\n", powerStateOrdinal);
    return IOPMAckImplied;
}

void IntelBluetoothFirmware::stop(IOService *provider)
{
    XYLog("Driver Stop()\n");
    PMstop();
    super::stop(provider);
}

IOService * IntelBluetoothFirmware::probe(IOService *provider, SInt32 *score)
{
    XYLog("Driver Probe()\n");
    if (!super::probe(provider, score)) {
        XYLog("super probe failed\n");
        return NULL;
    }
    m_pDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!m_pDevice) {
        XYLog("is not usb device\n");
        return NULL;
    }
    UInt16 vendorID = USBToHost16(m_pDevice->getDeviceDescriptor()->idVendor);
    UInt16 productID = USBToHost16(m_pDevice->getDeviceDescriptor()->idProduct);
    XYLog("name=%s, class=%s, vendorID=0x%04X, productID=0x%04X\n", m_pDevice->getName(), provider->metaClass->getClassName(), vendorID, productID);
    if (productID == 0x07dc || productID == 0x0a2a || productID == 0x0aa7) {
        currentType = kTypeGen1;
    } else if (productID == 0x0032 || productID == 0x0033) {
        currentType = kTypeGen3;
    } else {
        currentType = kTypeGen2;
    }
    m_pDevice = NULL;
    return this;
}
