# -------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
# --------------------------------------------------------------------------
# 
# Sample IoT Central Python code
#
# uses the Azure IoT Native Python Device SDK https://github.com/Azure/azure-iot-sdk-python/tree/master/azure-iot-device
#
# The code requires the use of Python version 3.7+ for optimal use
#
# install the libraries with: pip install azure-iot-device
#
# The capability model for IoT Central is in the file device-capability-model.json and should be loaded into the IoT Central application
# Views will need to be created to see the telemetry, properties and commands
#
# In the code add the necessary values for id_scope, group_symmetric_key, device_id, model_identity 
# and set use_websockets to true or false depending on if MQTT ports are blocked or not
#
# Features enabled in this sample:
#   * Supports DPS with cached DPS credentials (to file system) so a DPS call is not needed every connection
#   * if connection fails with cached credentials falls back to re-call DPS
#   * Supports device side registration of the device in IoT Central with auto-association to a device model
#   * Illustrates sending telemetry to IoT Central
#   * Illustrates sending reported properties
#   * Illustrates receiving desired properties and acknowledging them correctly back to IoT Central
#   * Illustrates receiving direct methods and acknowledging them and returning a return value
#   * Auto reconnects are handled by the Python device SDK
#

import os
import time
import asyncio
import base64
import hmac
import hashlib
import random
import json
import string

# uses the Azure IoT Device SDK for Python (Native Python libraries)
from azure.iot.device.aio import ProvisioningDeviceClient
from azure.iot.device.aio import IoTHubDeviceClient
from azure.iot.device import Message
from azure.iot.device import MethodResponse

# user configurable values
id_scope = ""
device_id = ""
group_symmetric_key = ""
model_identity = ""
use_websockets = True

# global counter
req_id = 1

# global variable declarations
provisioning_host = "global.azure-devices-provisioning.net"
device_client = None
terminate = False
use_cached_credentials = False

# derives a symmetric device key for a device id using the group symmetric key
def derive_device_key(device_id, group_symmetric_key):
    message = device_id.encode("utf-8")
    signing_key = base64.b64decode(group_symmetric_key.encode("utf-8"))
    signed_hmac = hmac.HMAC(signing_key, message, hashlib.sha256)
    device_key_encoded = base64.b64encode(signed_hmac.digest())
    return device_key_encoded.decode("utf-8")


# cache the DPS registration values to a file - could be stored in any nonvolatile storage (file, flash, etc.)
def write_dps_cache_to_file(cache):
    with open('dpsCache.json', 'w') as f:
        json.dump(cache, f, sort_keys=True)


# reads the cached DPS registration values from a file
def read_dps_cache_from_file():
    with open('dpsCache.json', 'r') as f:
        cache = json.load(f)
    return cache


# coroutine that sends telemetry on a set frequency until terminated
async def send_telemetry(device_client, send_frequency):
    while not terminate:
        global req_id
        payload = '{"cas_id": "%s", "req_id": %i, "cas_status": "%s"}' % (device_id, req_id, 'CAS_UPDATE_SUCCESSFUL')
        #payload = '{"cas_id": "a", "req_id": 1, "cas_status": "CAS_UPDATE_SUCCESSFUL"}' % (device_id, req_id, "CAS_UPDATE_SUCCESSFUL")
        print("Sending Telemetry Message: %s" % (payload))
        msg = Message(payload)
        try:
            await device_client.send_message(msg)
        except:
            print("Telemetry Error")
        print("Completed Sending Telemetry Message")
        req_id += 1
        await asyncio.sleep(send_frequency)


# coroutine that sends reported properties on a set frequency until terminated
async def send_reportedProperty(device_client, key, dataType, send_frequency):
    while not terminate:
        value = None
        if dataType == "bool":
            value = random.choice([True, False])
        elif dataType == "number":
            value = random.randrange(0, 100)
        elif dataType == "string":
            value = ''.join([random.choice(string.ascii_letters + string.digits) for n in range(32)])
        reported_payload = {key: {"value": value}}
        reported_payload2 = {"full_name": { "value": {"first_name": "Larry","last_name": "Jordan"}}}
        print(reported_payload)
        print(reported_payload2)
        try:
            await device_client.patch_twin_reported_properties(reported_payload)
            await asyncio.sleep(10)
            await device_client.patch_twin_reported_properties(reported_payload2)
        except:
            print("error")
        await asyncio.sleep(send_frequency)


# builds the acknowledge reported property for a desired property from IoT Central
async def desired_ack(json_data, status_code, status_text):
    # respond with IoT Central confirmation
    key = list(json_data.keys())[0]
    if list(json_data.keys())[0] == "$version":
        key = list(json_data.keys())[1]

    reported_payload = {key:{"value":json_data[key]['value'],"statusCode":status_code,"status":status_text,"desiredVersion":json_data['$version']}}
    print(reported_payload)
    await device_client.patch_twin_reported_properties(reported_payload)


# coroutine that handles desired properties from IoT Central (or hub) until terminated
async def twin_patch_handler(device_client):
    while not terminate:
        patch = await device_client.receive_twin_desired_properties_patch()
        print("the data in the desired properties patch was: {}".format(patch))
        # acknowledge the desired property back to IoT Central
        await desired_ack(patch, 200, "completed")


async def direct_method_handler(device_client):
    while not terminate:
        method_request = (
            await device_client.receive_method_request()
        )  # Wait for unknown method calls
        print("executing direct method: %s(%s)" % (method_request.name, method_request.payload))

        method_response = None
        if method_request.name == "echo":
            # send response - echo back the payload
            method_response = MethodResponse.create_from_method_request(method_request, 200, method_request.payload)
        else:
            # send bad request status code
            method_response = MethodResponse.create_from_method_request(method_request, 400, "unknown command")

        await device_client.send_method_response(method_response)


# main function: looks for cached DPS information in the file dpsCache.json and uses it to do a direct connection to the IoT hub.
# if the connection fails it falls back to DPS to get new credentials and caches those.  
# Reconnects are handled by the underlying Python device SDK.
async def main():
    global device_client

    random.seed()
    dps_registered = False
    connected = False
    connection_retry_count = 1

    while (not connected) and (connection_retry_count < 3):
        if use_cached_credentials and os.path.exists('dpsCache.json'):
            dps_cache = read_dps_cache_from_file()
            if dps_cache[2] == device_id:
                dps_registered = True
            else:
                os.remove('dpsCache.json')
                continue
        else:
            symmetric_key = derive_device_key(device_id, group_symmetric_key)

            provisioning_device_client = ProvisioningDeviceClient.create_from_symmetric_key(
                provisioning_host=provisioning_host,
                registration_id=device_id,
                id_scope=id_scope,
                symmetric_key=symmetric_key,
                websockets=use_websockets
            )

            provisioning_device_client.provisioning_payload = '{"iotcModelId":"%s"}' % (model_identity)
            registration_result = await provisioning_device_client.register()

            dps_cache = (symmetric_key, registration_result.registration_state.assigned_hub, registration_result.registration_state.device_id)
            if use_cached_credentials:
                write_dps_cache_to_file(dps_cache)

            print("The complete registration result is %s" % (registration_result.registration_state))
            if registration_result.status == "assigned":
                dps_registered = True

        if dps_registered:
            device_client = IoTHubDeviceClient.create_from_symmetric_key(
                symmetric_key=dps_cache[0],
                hostname=dps_cache[1],
                device_id=dps_cache[2],
                websockets=use_websockets
            )

        # connect
        try:
            await device_client.connect()
            connected = True
        except:
            print("Connection failed, retry %d of 3" % (connection_retry_count))
            if os.path.exists('dpsCache.json'):
                os.remove('dpsCache.json')
                dps_registered = False
            connection_retry_count = connection_retry_count + 1
        
    # add desired property listener
    twin_listener = asyncio.create_task(twin_patch_handler(device_client))

    # add direct method listener
    direct_method_listener = asyncio.create_task(direct_method_handler(device_client)) 

    # add tasks to send telemetry (every 5 seconds) and reported properties (every 20, 25, 30 seconds respectively)
    telemetry_loop = asyncio.create_task(send_telemetry(device_client, 5))
    reported_loop1 = asyncio.create_task(send_reportedProperty(device_client, "text", "string", 20))
    reported_loop2 = asyncio.create_task(send_reportedProperty(device_client, "boolean", "bool", 25))
    reported_loop3 = asyncio.create_task(send_reportedProperty(device_client, "number", "number", 30))

    #awit the tasks ending before exiting
    try:
        await asyncio.gather(twin_listener, direct_method_listener, telemetry_loop, reported_loop1, reported_loop2, reported_loop3) 

    except asyncio.CancelledError:
        pass # ignore the cancel actions on twin_listener and direct_method_listener

    # finally, disconnect
    print("Disconnecting from IoT Hub")
    await device_client.disconnect()

# start the main routine
if __name__ == "__main__":
    asyncio.run(main())

    # If using Python 3.6 or below, use the following code instead of asyncio.run(main()):
    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
    # loop.close()