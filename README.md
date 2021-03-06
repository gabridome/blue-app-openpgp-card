## GnuPG application: blue-app-gnupg

GnuPG application for Ledger Blue and Nano S

This application implements "The OpenPGP card" specification revision 3.0. This specification is available in *doc* directory and at https://g10code.com/p-card.html .

The application supports:
  - RSA with key up to 4096 bits 
  - ECDSA with secp256k1, secp256r1, brainpool 256r1 and brainpool 256t1 curves
  - EDDSA with Ed25519 curve 
  - ECDH with  secp256k1, secp256r1, brainpool 256r1, brainpool 256t1 and curve25519 curves


This release is in beta stage with known missing parts (see also Add-on) :

   * Ledger Blue support
   * Seed mode ON/OFF via apdu


## Installation

### NanoS

#### From source

  - use at least the Nano S SDK 1.3.1.4 on firmware 1.3.1
  - replace lib_stusb/STM32_USB_Device_Library/Class/CCID/src/usbd_ccid_if.c by the one provided in sdk/ directory

#### From Binary

Use the Chrome App "Ledger Manager". See https://www.ledgerwallet.com/apps/manager for details.

As "OpenPGP card" application is not fully released, click on "Show delevoppers items" on the bottom right corner. 

### Host

#### Linux

You have to have to add the NanoS to /etc/libccid_Info.plist

    In  <key>ifdVendorID</key>      add the entry  <string>0x2C97</string>
    In  <key>ifdProductID</key>     add the entry  <string>0x0001</string>
    In  <key>ifdFriendlyName</key>  add the entry  <string>Ledger Token</string>
  
This 3 entries must be added at the end of each list.

#### MAC

TODO

#### Windows

TODO


## Add-on

The GnuPG application implements the following addon:
  - serial modification
  - on screen reset
  - 3 independent key slots
  - seeded key generation

Technical specification is available in doc/gpgcard3.0-addon.rst

   
### Key slot

"The OpenPGP card" specification specifies:
  - 3 asymmetric keys : Signature, Decryption, Authentication
  - 1 symmetric key

The blue application allow you to store 3 different key sets, named slot. Each slot contains the above 4 keys.
You can choose the active slot on the main screen.
When installed the default slot is "1". You can change it in settings.

   
### seeded key generation

A seeded mode is implemented in order to restore private keys on a new token.
In this mode key material is generated from the global token seeded.

Please consider SEED mode as experimental.

More details to come... 

### On screen reset

The application can be reset as if it was fresh installed. In settings, choose reset and confirm.

