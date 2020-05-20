# IoT Central Device Training
## Module 04 - Enabling Capabilities on the RPi

### This Text is from the Raspberry Pi Foundations Documentation

## raspi-config
This page describes the console based raspi-config application. If you are using the Raspberry Pi desktop then you can use the graphical Raspberry Pi Configuration application from the Preferences menu to configure your Raspberry Pi.

raspi-config is the Raspberry Pi configuration tool originally written by Alex Bradbury. It targets Raspbian.

### Usage
You will be shown raspi-config on first booting into Raspbian. To open the configuration tool after this, simply run the following from the command line:

sudo raspi-config

The sudo is required because you will be changing files that you do not own as the pi user.
You should see a blue screen with options in a grey box in the centre, like so:


It has the following options available:

```
┌───────────────────┤ Raspberry Pi Software Configuration Tool (raspi-config) ────────────────────┐
│                                                                                                 │
│        1 Change User Password Change password for the current user                              │
│        2 Network Options      Configure network settings                                        │
│        3 Boot Options         Configure options for start-up                                    │
│        4 Localisation Options Set up language and regional settings to match your location      │
│        5 Interfacing Options  Configure connections to peripherals                              │
│        6 Overclock            Configure overclocking for your Pi                                │
│        7 Advanced Options     Configure advanced settings                                       │
│        8 Update               Update this tool to the latest version                            │
│        9 About raspi-config   Information about this configuration tool                         │
│                                                                                                 │
│                                                                                                 │
│                                                                                                 │
│                           <Select>                           <Finish>                           │
│                                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## [NEXT: Module 05 - Connecting using SSH to your Raspberry Pi](../Module05/README.md)