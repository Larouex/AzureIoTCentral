# IoT Central Device Training
## Module 16 - Connecting the Things Network to Azure IoT Central Via Device Bridge

* [LINK: DOCS IoT Central Device Bridge](https://docs.microsoft.com/en-us/azure/iot-central/core/howto-build-iotc-device-bridge)
* [LINK: IoT Central Device Bridge](https://github.com/Azure/iotc-device-bridge)
* [LINK: The Things Network](https://www.thethingsnetwork.org/)
* [LINK: Yoder at Azure IoT Central](https://yoder-observer.azureiotcentral.com/)

``` javascript
function Decoder(bytes, port) {
    function bytesToFloat(bytes, decimalPlaces) {
        var bits = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
        var sign = (bits >>> 31 === 0) ? 1.0 : -1.0;
        var e = bits >>> 23 & 0xff;
        var m = (e === 0) ? (bits & 0x7fffff) << 1 : (bits & 0x7fffff) | 0x800000;
        var f = Math.round((sign * m * Math.pow(2, e - 150)) * Math.pow(10, decimalPlaces)) / Math.pow(10, decimalPlaces);
        return f;
    }

    function bytesToInt32(bytes, signed) {
        var bits = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        var sign = 1;

        if (signed && bits >>> 31 === 1) {
            sign = -1;
            bits = bits & 0x7FFFFFFF;
        }

        return bits * sign;
    }

    function bytesToShort(bytes, signed) {
        var bits = bytes[0] | (bytes[1] << 8);
        var sign = 1;

        if (signed && bits >>> 15 === 1) {
            sign = -1;
            bits = bits & 0x7FFF;
        }

        return bits * sign;
    }

    return {
        at: bytesToFloat(bytes.slice(0, 4), 2),
        fb: bytesToFloat(bytes.slice(4, 8), 2),
        wb: bytesToFloat(bytes.slice(8, 12), 2),
        lb: bytesToFloat(bytes.slice(12, 16), 2),
        rb: bytesToFloat(bytes.slice(16, 20), 2),
        lf: bytesToFloat(bytes.slice(20, 24), 2),
        rf: bytesToFloat(bytes.slice(24, 28), 2)
    };
}
```