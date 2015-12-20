/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XBee_h
#define XBee_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <inttypes.h>

#define SERIES_1

// set to ATAP value of XBee. AP=2 is recommended
#define ATAP 2

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

// This value determines the size of the byte array for receiving RX packets
// Most users won't be dealing with packets this large so you can adjust this
// value to reduce memory consumption. But, remember that
// if a RX packet exceeds this size, it cannot be parsed!

// This value is determined by the largest packet size (Start Byte + Length Bytes + API + 100 byte payload + 64-bit address + option byte and rssi byte) of a series 1 radio
#define MAX_FRAME_DATA_SIZE 114

#define BROADCAST_ADDRESS 0xffff
#define ZB_BROADCAST_ADDRESS 0xfffe

// the non-variable length of the frame data (not including frame id or api id or variable data size (e.g. payload, at command set value)
#define ZB_TX_API_LENGTH 12
#define TX_16_API_LENGTH 3
#define TX_64_API_LENGTH 9
#define AT_COMMAND_API_LENGTH 2
#define REMOTE_AT_COMMAND_API_LENGTH 13
// start/length(2)/api/frameid/checksum bytes
#define PACKET_OVERHEAD_LENGTH 6
// api is always the third byte in packet
#define API_ID_INDEX 3

// frame position of rssi byte
#define RX_16_RSSI_OFFSET 2
#define RX_64_RSSI_OFFSET 8

#define DEFAULT_FRAME_ID 1
#define NO_RESPONSE_FRAME_ID 0

// TODO put in tx16 class
#define ACK_OPTION 0
#define DISABLE_ACK_OPTION 1
#define DISABE_ACK_OPTION_AND_ROUTE_DISCOVERY 0x11
#define BROADCAST_OPTION 4

// not everything is implemented!
/**
 * Api Id constants
 */
#define TX_64_REQUEST 0x0
#define TX_16_REQUEST 0x1
#define AT_COMMAND_REQUEST 0x08
#define AT_COMMAND_QUEUE_REQUEST 0x09
#define REMOTE_AT_REQUEST 0x17
#define ZB_TX_REQUEST 0x10
#define ZB_EXPLICIT_TX_REQUEST 0x11
#define RX_64_RESPONSE 0x80
#define RX_16_RESPONSE 0x81
#define RX_64_IO_RESPONSE 0x82
#define RX_16_IO_RESPONSE 0x83
#define AT_RESPONSE 0x88
#define TX_STATUS_RESPONSE 0x89
#define MODEM_STATUS_RESPONSE 0x8a
#define AT_COMMAND_RESPONSE 0x88
#define REMOTE_AT_COMMAND_RESPONSE 0x97

/**
 * TX STATUS constants
 */
#define	SUCCESS 0x0
#define CCA_FAILURE 0x2
#define INVALID_DESTINATION_ENDPOINT_SUCCESS 0x15
#define	NETWORK_ACK_FAILURE 0x21
#define NOT_JOINED_TO_NETWORK 0x22
#define	SELF_ADDRESSED 0x23
#define ADDRESS_NOT_FOUND 0x24
#define ROUTE_NOT_FOUND 0x25
#define PAYLOAD_TOO_LARGE 0x74

#define AT_OK 0
#define AT_ERROR  1
#define AT_INVALID_COMMAND 2
#define AT_INVALID_PARAMETER 3
#define AT_NO_RESPONSE 4

#define NO_ERROR 0
#define CHECKSUM_FAILURE 1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH 2
#define UNEXPECTED_START_BYTE 3

/**
 * The super class of all XBee responses (RX packets)
 * Users should never attempt to create an instance of this class; instead
 * create an instance of a subclass
 * It is recommend to reuse subclasses to conserve memory
 */
class XBeeResponse {
	public:
		//static const int MODEM_STATUS = 0x8a;
		/**
		 * Default constructor
		 */
		XBeeResponse();
		/**
		 * Returns Api Id of the response
		 */
		uint8_t getApiId() const;
		void setApiId(uint8_t apiId);
		/**
		 * Returns the MSB length of the packet
		 */
		uint8_t getMsbLength() const;
		void setMsbLength(uint8_t msbLength);
		/**
		 * Returns the LSB length of the packet
		 */
		uint8_t getLsbLength() const;
		void setLsbLength(uint8_t lsbLength);
		/**
		 * Returns the packet checksum
		 */
		uint8_t getChecksum() const;
		void setChecksum(uint8_t checksum);
		/**
		 * Returns the length of the frame data: all bytes after the api id, and prior to the checksum
		 * Note up to release 0.1.2, this was incorrectly including the checksum in the length.
		 */
		uint8_t getFrameDataLength() const;
		void setFrameData(uint8_t* frameDataPtr);
		/**
		 * Returns the buffer that contains the response.
		 * Starts with byte that follows API ID and includes all bytes prior to the checksum
		 * Length is specified by getFrameDataLength()
		 * Note: Unlike Digi's definition of the frame data, this does not start with the API ID..
		 * The reason for this is all responses include an API ID, whereas my frame data
		 * includes only the API specific data.
		 */
		uint8_t* getFrameData() const;

		void setFrameLength(uint8_t frameLength);
		// to support future 65535 byte packets I guess
		/**
		 * Returns the length of the packet
		 */
		uint16_t getPacketLength() const;
		/**
		 * Resets the response to default values
		 */
		void reset();
		/**
		 * Initializes the response
		 */
		void init();

		/**
		 * Call with instance of TxStatusResponse only if getApiId() == TX_STATUS_RESPONSE
		 */
		void getTxStatusResponse(XBeeResponse &response) const;
		/**
		 * Call with instance of Rx16Response only if getApiId() == RX_16_RESPONSE
		 */
		void getRx16Response(XBeeResponse &response) const;
		/**
		 * Call with instance of Rx64Response only if getApiId() == RX_64_RESPONSE
		 */
		void getRx64Response(XBeeResponse &response) const;

		/**
		 * Call with instance of AtCommandResponse only if getApiId() == AT_COMMAND_RESPONSE
		 */
		void getAtCommandResponse(XBeeResponse &responses);
		/**
		 * Call with instance of RemoteAtCommandResponse only if getApiId() == REMOTE_AT_COMMAND_RESPONSE
		 */
		void getRemoteAtCommandResponse(XBeeResponse &response) const;
		/**
		 * Call with instance of ModemStatusResponse only if getApiId() == MODEM_STATUS_RESPONSE
		 */
		void getModemStatusResponse(XBeeResponse &response) const;
		/**
		 * Returns true if the response has been successfully parsed and is complete and ready for use
		 */
		bool isAvailable() const;
		void setAvailable(const bool complete);
		/**
		 * Returns true if the response contains errors
		 */
		bool isError() const;
		/**
		 * Returns an error code, or zero, if successful.
		 * Error codes include: CHECKSUM_FAILURE, PACKET_EXCEEDS_BYTE_ARRAY_LENGTH, UNEXPECTED_START_BYTE
		 */
		uint8_t getErrorCode() const;
		void setErrorCode(const uint8_t errorCode);
		void setSerial(Stream &serial);
		Stream* getSerial() const;
	protected:
		// pointer to frameData
		uint8_t* _frameDataPtr;
		Stream* _serial;
	private:
		void setCommon(XBeeResponse &target) const;
		uint8_t _apiId;
		uint8_t _msbLength;
		uint8_t _lsbLength;
		uint8_t _checksum;
		uint8_t _frameLength;
		bool _complete;
		uint8_t _errorCode;
};

class XBeeAddress {
	public:
		XBeeAddress();
};

/**
 * Represents a 64-bit XBee Address
 */
class XBeeAddress64: public XBeeAddress {
	public:
		XBeeAddress64(uint32_t msb, uint32_t lsb);
		XBeeAddress64();
		uint32_t getMsb() const;
		uint32_t getLsb() const;
		void setMsb(uint32_t msb);
		void setLsb(uint32_t lsb);
		bool equals(const XBeeAddress64& x) const;
		void printAddress(Stream* _serial) const;
		void printAddressASCII(Stream* _serial) const;
		bool operator<(const XBeeAddress64& addr) const;
		//bool operator==(XBeeAddress64 addr);
		//bool operator!=(XBeeAddress64 addr);
	private:
		uint32_t _msb = 0;
		uint32_t _lsb = 0;

};

/**
 * This class is extended by all Responses that include a frame id
 */
class FrameIdResponse: public XBeeResponse {
	public:
		FrameIdResponse();
		uint8_t getFrameId() const;
	private:
		uint8_t _frameId = 0;

};

/**
 * Common functionality for both Series 1 and 2 data RX data packets
 */
class RxDataResponse: public XBeeResponse {
	public:
		RxDataResponse();
		virtual ~RxDataResponse();
		/**
		 * Returns the specified index of the payload.  The index may be 0 to getDataLength() - 1
		 * This method is deprecated; use uint8_t* getData()
		 */
		uint8_t getData(const int index) const;
		/**
		 * Returns the payload array.  This may be accessed from index 0 to getDataLength() - 1
		 */
		uint8_t* getData() const;
		/**
		 * Returns the length of the payload
		 */
		virtual uint8_t getDataLength() const = 0;
		/**
		 * Returns the position in the frame data where the data begins
		 */
		virtual uint8_t getDataOffset() const = 0;
};

// getResponse to return the proper subclass:
// we maintain a pointer to each type of response, when a response is parsed, it is allocated only if NULL
// can we allocate an object in a function?

/**
 * Represents a Series 1 TX Status packet
 */
class TxStatusResponse: public FrameIdResponse {
	public:
		TxStatusResponse();
		uint8_t getStatus() const;
		bool isSuccess() const;
		uint8_t getTransmitCount();
};

/**
 * Represents a Series 1 RX packet
 */
class RxResponse: public RxDataResponse {
	public:
		RxResponse();
		virtual ~RxResponse();
		// remember rssi is negative but this is unsigned byte so it's up to you to convert
		uint8_t getRssi() const;
		uint8_t getOption() const;
		bool isAddressBroadcast() const;
		bool isPanBroadcast() const;
		uint8_t getDataLength() const;
		uint8_t getDataOffset() const;
		virtual uint8_t getRssiOffset() const = 0;
};

/**
 * Represents a Series 1 64-bit address RX packet
 */
class Rx64Response: public RxResponse {
	public:
		Rx64Response();
		virtual ~Rx64Response();
		uint8_t getRssiOffset() const;
		XBeeAddress64 getRemoteAddress64() const;
		void setRemoteAddress64(const XBeeAddress64& remoteAddress);
	private:
		XBeeAddress64 _remoteAddress;
};

/**
 * Represents an AT Command RX packet
 */
class AtCommandResponse: public FrameIdResponse {
	public:
		AtCommandResponse();
		/**
		 * Returns an array containing the two character command
		 */
		uint8_t* getCommand() const;
		/**
		 * Returns the command status code.
		 * Zero represents a successful command
		 */
		uint8_t getStatus() const;
		/**
		 * Returns an array containing the command value.
		 * This is only applicable to query commands.
		 */
		uint8_t* getValue() const;
		/**
		 * Returns the length of the command value array.
		 */
		uint8_t getValueLength() const;
		/**
		 * Returns true if status equals AT_OK
		 */
		bool isOk() const;
};

/**
 * Represents a Remote AT Command RX packet
 */
class RemoteAtCommandResponse: public AtCommandResponse {
	public:
		RemoteAtCommandResponse();
		/**
		 * Returns an array containing the two character command
		 */
		uint8_t* getCommand() const;
		/**
		 * Returns the command status code.
		 * Zero represents a successful command
		 */
		uint8_t getStatus() const;
		/**
		 * Returns an array containing the command value.
		 * This is only applicable to query commands.
		 */
		uint8_t* getValue() const;
		/**
		 * Returns the length of the command value array.
		 */
		uint8_t getValueLength() const;
		/**
		 * Returns the 16-bit address of the remote radio
		 */
		uint16_t getRemoteAddress16() const;
		/**
		 * Returns the 64-bit address of the remote radio
		 */
		XBeeAddress64& getRemoteAddress64();
		/**
		 * Returns true if command was successful
		 */
		bool isOk() const;
	private:
		XBeeAddress64 _remoteAddress64;
};

/**
 * Super class of all XBee requests (TX packets)
 * Users should never create an instance of this class; instead use an subclass of this class
 * It is recommended to reuse Subclasses of the class to conserve memory
 * <p/>
 * This class allocates a buffer to
 */
class XBeeRequest {
	public:
		/**
		 * Constructor
		 * TODO make protected
		 */
		XBeeRequest(const uint8_t apiId, const uint8_t frameId);
		virtual ~XBeeRequest();
		/**
		 * Sets the frame id.  Must be between 1 and 255 inclusive to get a TX status response.
		 */
		void setFrameId(uint8_t frameId);
		/**
		 * Returns the frame id
		 */
		uint8_t getFrameId() const;
		/**
		 * Returns the API id
		 */
		uint8_t getApiId() const;
		// setting = 0 makes this a pure virtual function, meaning the subclass must implement, like abstract in java
		/**
		 * Starting after the frame id (pos = 0) and up to but not including the checksum
		 * Note: Unlike Digi's definition of the frame data, this does not start with the API ID.
		 * The reason for this is the API ID and Frame ID are common to all requests, whereas my definition of
		 * frame data is only the API specific data.
		 */
		virtual uint8_t getFrameData(uint8_t pos) const = 0;
		/**
		 * Returns the size of the api frame (not including frame id or api id or checksum).
		 */
		virtual uint8_t getFrameDataLength() const = 0;
		//void reset();
	protected:
		void setApiId(uint8_t apiId);
	private:
		uint8_t _apiId;
		uint8_t _frameId;
};

// TODO add reset/clear method since responses are often reused
/**
 * Primary interface for communicating with an XBee Radio.
 * This class provides methods for sending and receiving packets with an XBee radio via the serial port.
 * The XBee radio must be configured in API (packet) mode (AP=2)
 * in order to use this software.
 * <p/>
 * Since this code is designed to run on a microcontroller, with only one thread, you are responsible for reading the
 * data off the serial buffer in a timely manner.  This involves a call to a variant of readPacket(...).
 * If your serial port is receiving data faster than you are reading, you can expect to lose packets.
 * Arduino only has a 128 byte serial buffer so it can easily overflow if two or more packets arrive
 * without a call to readPacket(...)
 * <p/>
 * In order to conserve resources, this class only supports storing one response packet in memory at a time.
 * This means that you must fully consume the packet prior to calling readPacket(...), because calling
 * readPacket(...) overwrites the previous response.
 * <p/>
 * This class creates an array of size MAX_FRAME_DATA_SIZE for storing the response packet.  You may want
 * to adjust this value to conserve memory.
 *
 * \author Andrew Rapp
 */
class XBee {
	public:
		XBee();
		/**
		 * Reads all available serial bytes until a packet is parsed, an error occurs, or the buffer is empty.
		 * You may call <i>xbee</i>.getResponse().isAvailable() after calling this method to determine if
		 * a packet is ready, or <i>xbee</i>.getResponse().isError() to determine if
		 * a error occurred.
		 * <p/>
		 * This method should always return quickly since it does not wait for serial data to arrive.
		 * You will want to use this method if you are doing other timely stuff in your loop, where
		 * a delay would cause problems.
		 * NOTE: calling this method resets the current response, so make sure you first consume the
		 * current response
		 */
		void readPacket(bool debug);
		/**
		 * Waits a maximum of <i>timeout</i> milliseconds for a response packet before timing out; returns true if packet is read.
		 * Returns false if timeout or error occurs.
		 */
		bool readPacket(int timeout, bool debug);
		/**
		 * Reads until a packet is received or an error occurs.
		 * Caution: use this carefully since if you don't get a response, your Arduino code will hang on this
		 * call forever!! often it's better to use a timeout: readPacket(int)
		 */
		void readPacketUntilAvailable();
		/**
		 * Starts the serial connection on the specified serial port
		 */

		bool readPacketNoTimeout(bool debug);

		void begin(Stream &serial);
		void getResponse(XBeeResponse &response) const;
		/**
		 * Returns a reference to the current response
		 * Note: once readPacket is called again this response will be overwritten!
		 */
		XBeeResponse& getResponse();
		/**
		 * Sends a XBeeRequest (TX packet) out the serial port
		 */

		void packageSend(const XBeeRequest &request);
		void packageByte(uint8_t buffer[], const uint8_t b, const bool escape, uint8_t &index);

		void send(const XBeeRequest &request);
		//uint8_t sendAndWaitForResponse(XBeeRequest &request, int timeout);
		/**
		 * Returns a sequential frame id between 1 and 255
		 */
		uint8_t getNextFrameId();
		/**
		 * Specify the serial port.  Only relevant for Arduinos that support multiple serial ports (e.g. Mega)
		 */
		void setSerial(Stream &serial);
	private:
		bool available() const;
		uint8_t read() const;
		void flush();
		void write(uint8_t val);
		void write(const uint8_t *buffer, size_t size);
		void sendByte(uint8_t b, bool escape);
		void resetResponse();
		XBeeResponse _response;
		bool _escape;
		// current packet position for response.  just a state variable for packet parsing and has no relevance for the response otherwise
		uint8_t _pos;
		// last byte read
		uint8_t b = 0;
		uint8_t _checksumTotal;
		uint8_t _nextFrameId;
		// buffer for incoming RX packets.  holds only the api specific frame data, starting after the api id byte and prior to checksum
		uint8_t _responseFrameData[MAX_FRAME_DATA_SIZE];
		Stream* _serial;
};

/**
 * All TX packets that support payloads extend this class
 */
class PayloadRequest: public XBeeRequest {
	public:
		PayloadRequest(const uint8_t apiId, const uint8_t frameId, uint8_t *payload, const uint8_t payloadLength);
		virtual ~PayloadRequest();
		/**
		 * Returns the payload of the packet, if not null
		 */
		uint8_t* getPayload() const;
		/**
		 * Sets the payload array
		 */
		void setPayload(uint8_t* payloadPtr);
		/**
		 * Returns the length of the payload array, as specified by the user.
		 */
		uint8_t getPayloadLength() const;
		/**
		 * Sets the length of the payload to include in the request.  For example if the payload array
		 * is 50 bytes and you only want the first 10 to be included in the packet, set the length to 10.
		 * Length must be <= to the array length.
		 */
		void setPayloadLength(const uint8_t payloadLength);
	private:
		uint8_t* _payloadPtr;
		uint8_t _payloadLength;
};

/**
 * Represents a Series 1 TX packet that corresponds to Api Id: TX_64_REQUEST
 *
 * Be careful not to send a data array larger than the max packet size of your radio.
 * This class does not perform any validation of packet size and there will be no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 100 bytes is the maximum, although that could change in future firmware.
 */
class Tx64Request: public PayloadRequest {
	public:
		Tx64Request(const XBeeAddress64 &addr64, const uint8_t option, uint8_t *payload, const uint8_t payloadLength,
				uint8_t frameId);
		/**
		 * Creates a unicast Tx64Request with the ACK option and DEFAULT_FRAME_ID
		 */
		Tx64Request(const XBeeAddress64 &addr64, uint8_t *payload, const uint8_t payloadLength);
		/**
		 * Creates a default instance of this class.  At a minimum you must specify
		 * a payload, payload length and a destination address before sending this request.
		 */
		Tx64Request();
		virtual ~Tx64Request();
		XBeeAddress64& getAddress64();
		void setAddress64(const XBeeAddress64& addr64);
		// TODO move option to superclass
		uint8_t getOption() const;
		void setOption(const uint8_t option);
		uint8_t getFrameData(const uint8_t pos) const;
		uint8_t getFrameDataLength() const;
	private:
		XBeeAddress64 _addr64;
		uint8_t _option = 0;
};

/**
 * Represents an AT Command TX packet
 * The command is used to configure the serially connected XBee radio
 */
class AtCommandRequest: public XBeeRequest {
	public:
		AtCommandRequest();
		AtCommandRequest(uint8_t *command);
		AtCommandRequest(uint8_t *command, uint8_t *commandValue, uint8_t commandValueLength);
		virtual ~AtCommandRequest();
		uint8_t getFrameData(const uint8_t pos) const;
		uint8_t getFrameDataLength() const;
		uint8_t* getCommand() const;
		void setCommand(uint8_t* command);
		uint8_t* getCommandValue() const;
		void setCommandValue(uint8_t* command);
		uint8_t getCommandValueLength() const;
		void setCommandValueLength(const uint8_t length);
		/**
		 * Clears the optional commandValue and commandValueLength so that a query may be sent
		 */
		void clearCommandValue();
		//void reset();
	private:
		uint8_t *_command;
		uint8_t *_commandValue;
		uint8_t _commandValueLength;
};

/**
 * Represents an Remote AT Command TX packet
 * The command is used to configure a remote XBee radio
 */
class RemoteAtCommandRequest: public AtCommandRequest {
	public:
		RemoteAtCommandRequest();
		/**
		 * Creates a RemoteAtCommandRequest with 16-bit address to set a command.
		 * 64-bit address defaults to broadcast and applyChanges is true.
		 */
		RemoteAtCommandRequest(uint16_t remoteAddress16, uint8_t *command, uint8_t *commandValue,
				uint8_t commandValueLength);
		/**
		 * Creates a RemoteAtCommandRequest with 16-bit address to query a command.
		 * 64-bit address defaults to broadcast and applyChanges is true.
		 */
		RemoteAtCommandRequest(uint16_t remoteAddress16, uint8_t *command);
		/**
		 * Creates a RemoteAtCommandRequest with 64-bit address to set a command.
		 * 16-bit address defaults to broadcast and applyChanges is true.
		 */
		RemoteAtCommandRequest(XBeeAddress64 &remoteAddress64, uint8_t *command, uint8_t *commandValue,
				uint8_t commandValueLength);
		/**
		 * Creates a RemoteAtCommandRequest with 16-bit address to query a command.
		 * 16-bit address defaults to broadcast and applyChanges is true.
		 */
		RemoteAtCommandRequest(XBeeAddress64 &remoteAddress64, uint8_t *command);
		uint16_t getRemoteAddress16() const;
		void setRemoteAddress16(const uint16_t remoteAddress16);
		XBeeAddress64& getRemoteAddress64();
		void setRemoteAddress64(const XBeeAddress64 &remoteAddress64);
		bool getApplyChanges() const;
		void setApplyChanges(const bool applyChanges);
		uint8_t getFrameData(const uint8_t pos) const;
		uint8_t getFrameDataLength() const;
		static XBeeAddress64 broadcastAddress64;
//	static uint16_t broadcast16Address;
	private:
		XBeeAddress64 _remoteAddress64;
		uint16_t _remoteAddress16;
		bool _applyChanges;
};

#endif //XBee_h
