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

#include "XBee.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "HardwareSerial.h"

#define DEBUG true

XBeeResponse::XBeeResponse() {

}

uint8_t XBeeResponse::getApiId() const {
	return _apiId;
}

void XBeeResponse::setApiId(const uint8_t apiId) {
	_apiId = apiId;
}

uint8_t XBeeResponse::getMsbLength() const {
	return _msbLength;
}

void XBeeResponse::setMsbLength(const uint8_t msbLength) {
	_msbLength = msbLength;
}

uint8_t XBeeResponse::getLsbLength() const {
	return _lsbLength;
}

void XBeeResponse::setLsbLength(const uint8_t lsbLength) {
	_lsbLength = lsbLength;
}

uint8_t XBeeResponse::getChecksum() const {
	return _checksum;
}

void XBeeResponse::setChecksum(const uint8_t checksum) {
	_checksum = checksum;
}

uint8_t XBeeResponse::getFrameDataLength() const {
	return _frameLength;
}

void XBeeResponse::setFrameLength(const uint8_t frameLength) {
	_frameLength = frameLength;
}

bool XBeeResponse::isAvailable() const {
	return _complete;
}

void XBeeResponse::setAvailable(const bool complete) {
	_complete = complete;
}

bool XBeeResponse::isError() const {
	return _errorCode > 0;
}

uint8_t XBeeResponse::getErrorCode() const {
	return _errorCode;
}

void XBeeResponse::setErrorCode(const uint8_t errorCode) {
	_errorCode = errorCode;
}

void XBeeResponse::setSerial(Stream &serial) {
	_serial = &serial;
}

Stream* XBeeResponse::getSerial() const {
	return _serial;
}

// copy common fields from xbee response to target response
void XBeeResponse::setCommon(XBeeResponse &target) const {
	target.setApiId(getApiId());
	target.setAvailable(isAvailable());
	target.setChecksum(getChecksum());
	target.setErrorCode(getErrorCode());
	target.setFrameLength(getFrameDataLength());
	target.setMsbLength(getMsbLength());
	target.setLsbLength(getLsbLength());
	target.setSerial(*(getSerial()));
}

RxResponse::RxResponse() :
		RxDataResponse() {

}

RxResponse::~RxResponse() {

}

XBeeAddress64 Rx64Response::getRemoteAddress64() const {
	return _remoteAddress;
}

void Rx64Response::setRemoteAddress64(const XBeeAddress64& remoteAddress) {
	_remoteAddress = remoteAddress;
}

Rx64Response::Rx64Response() :
		RxResponse() {
	_remoteAddress = XBeeAddress64();
}

Rx64Response::~Rx64Response() {

}

TxStatusResponse::TxStatusResponse() :
		FrameIdResponse() {

}

uint8_t TxStatusResponse::getStatus() const {
	return getFrameData()[1];
}

bool TxStatusResponse::isSuccess() const {
	return getStatus() == SUCCESS;
}

void XBeeResponse::getTxStatusResponse(XBeeResponse &txResponse) const {

	TxStatusResponse* txStatus = static_cast<TxStatusResponse*>(&txResponse);

	// pass pointer array to subclass
	txStatus->setFrameData(getFrameData());
	setCommon(txResponse);
}

uint8_t RxResponse::getRssi() const {
	return getFrameData()[getRssiOffset()];
}

uint8_t RxResponse::getOption() const {
	return getFrameData()[getRssiOffset() + 1];
}

bool RxResponse::isAddressBroadcast() const {
	return (getOption() & 2) == 2;
}

bool RxResponse::isPanBroadcast() const {
	return (getOption() & 4) == 4;
}

uint8_t RxResponse::getDataLength() const {
	return getPacketLength() - getDataOffset() - 1;
}

uint8_t RxResponse::getDataOffset() const {
	return getRssiOffset() + 2;
}

uint8_t Rx64Response::getRssiOffset() const {
	return RX_64_RSSI_OFFSET;
}

void XBeeResponse::getRx64Response(XBeeResponse &rx64Response) const {

	Rx64Response* rx64 = static_cast<Rx64Response*>(&rx64Response);

	// pass pointer array to subclass
	rx64->setFrameData(getFrameData());
	setCommon(rx64Response);

	XBeeAddress64 remoteAddress;

	remoteAddress.setMsb(
			(uint32_t(getFrameData()[0]) << 24) + (uint32_t(getFrameData()[1]) << 16)
					+ (uint16_t(getFrameData()[2]) << 8) + getFrameData()[3]);
	remoteAddress.setLsb(
			(uint32_t(getFrameData()[4]) << 24) + (uint32_t(getFrameData()[5]) << 16)
					+ (uint16_t(getFrameData()[6]) << 8) + getFrameData()[7]);

	rx64->setRemoteAddress64(remoteAddress);

}

RemoteAtCommandResponse::RemoteAtCommandResponse() :
		AtCommandResponse() {

}

uint8_t* RemoteAtCommandResponse::getCommand() const {
	return getFrameData() + 11;
}

uint8_t RemoteAtCommandResponse::getStatus() const {
	return getFrameData()[13];
}

bool RemoteAtCommandResponse::isOk() const {
	// weird c++ behavior.  w/o this method, it calls AtCommandResponse::isOk(), which calls the AtCommandResponse::getStatus, not this.getStatus!!!
	return getStatus() == AT_OK;
}

uint8_t RemoteAtCommandResponse::getValueLength() const {
	return getFrameDataLength() - 14;
}

uint8_t* RemoteAtCommandResponse::getValue() const {
	if (getValueLength() > 0) {
		// value is only included for query commands.  set commands does not return a value
		return getFrameData() + 14;
	}

	return NULL;
}

uint16_t RemoteAtCommandResponse::getRemoteAddress16() const {
	return uint16_t((getFrameData()[9] << 8) + getFrameData()[10]);
}

XBeeAddress64& RemoteAtCommandResponse::getRemoteAddress64() {
	return _remoteAddress64;
}

void XBeeResponse::getRemoteAtCommandResponse(XBeeResponse &response) const {

	RemoteAtCommandResponse* at = static_cast<RemoteAtCommandResponse*>(&response);

	// pass pointer array to subclass
	at->setFrameData(getFrameData());
	setCommon(response);

	at->getRemoteAddress64().setMsb(
			(uint32_t(getFrameData()[1]) << 24) + (uint32_t(getFrameData()[2]) << 16)
					+ (uint16_t(getFrameData()[3]) << 8) + getFrameData()[4]);
	at->getRemoteAddress64().setLsb(
			(uint32_t(getFrameData()[5]) << 24) + (uint32_t(getFrameData()[6]) << 16)
					+ (uint16_t(getFrameData()[7]) << 8) + (getFrameData()[8]));

}

RxDataResponse::RxDataResponse() :
		XBeeResponse() {

}

RxDataResponse::~RxDataResponse() {

}
uint8_t RxDataResponse::getData(const int index) const {
	return getFrameData()[getDataOffset() + index];
}

uint8_t* RxDataResponse::getData() const {
	return getFrameData() + getDataOffset();
}

FrameIdResponse::FrameIdResponse() {

}

uint8_t FrameIdResponse::getFrameId() const {
	return getFrameData()[0];
}

AtCommandResponse::AtCommandResponse() {

}

uint8_t* AtCommandResponse::getCommand() const {
	return getFrameData() + 1;
}

uint8_t AtCommandResponse::getStatus() const {
	return getFrameData()[3];
}

uint8_t AtCommandResponse::getValueLength() const {
	return getFrameDataLength() - 4;
}

uint8_t* AtCommandResponse::getValue() const {
	if (getValueLength() > 0) {
		// value is only included for query commands.  set commands does not return a value
		return getFrameData() + 4;
	}

	return NULL;
}

bool AtCommandResponse::isOk() const {
	return getStatus() == AT_OK;
}

void XBeeResponse::getAtCommandResponse(XBeeResponse &atCommandResponse) {

	AtCommandResponse* at = static_cast<AtCommandResponse*>(&atCommandResponse);

	// pass pointer array to subclass
	at->setFrameData(getFrameData());
	setCommon(atCommandResponse);
}

uint16_t XBeeResponse::getPacketLength() const {
	return ((_msbLength << 8) & 0xff) + (_lsbLength & 0xff);
}

uint8_t* XBeeResponse::getFrameData() const {
	return _frameDataPtr;
}

void XBeeResponse::setFrameData(uint8_t* frameDataPtr) {
	_frameDataPtr = frameDataPtr;
}

void XBeeResponse::init() {
	_complete = false;
	_errorCode = NO_ERROR;
	_checksum = 0;
}

void XBeeResponse::reset() {
	init();
	_apiId = 0;
	_msbLength = 0;
	_lsbLength = 0;
	_checksum = 0;
	_frameLength = 0;

	_errorCode = NO_ERROR;
}

void XBee::resetResponse() {
	_pos = 0;
	_escape = false;
	_checksumTotal = 0;
	_response.reset();
}

XBee::XBee() :
		_response(XBeeResponse()) {
	_pos = 0;
	_escape = false;
	_checksumTotal = 0;
	_nextFrameId = 0;

	_response.init();
	_response.setFrameData(_responseFrameData);
	// Contributed by Paul Stoffregen for Teensy support
#if defined(__AVR_ATmega32U4__) || defined(__MK20DX128__)
	_serial = &Serial1;
#else
	_serial = &Serial;
#endif
}

uint8_t XBee::getNextFrameId() {

	_nextFrameId++;

	if (_nextFrameId == 0) {
		// can't send 0 because that disables status response
		_nextFrameId = 1;
	}

	return _nextFrameId;
}

// Support for SoftwareSerial. Contributed by Paul Stoffregen
void XBee::begin(Stream &serial) {
	_serial = &serial;
}

void XBee::setSerial(Stream &serial) {
	_serial = &serial;
}

bool XBee::available() const {
	return _serial->available();
}

uint8_t XBee::read() const {
	return _serial->read();
}

void XBee::flush() {
	_serial->flush();
}

void XBee::write(const uint8_t val) {
	_serial->write(val);
}

void XBee::write(const uint8_t *buffer, size_t size) {
	_serial->write(buffer, size);
}

XBeeResponse& XBee::getResponse() {
	return _response;
}

void XBee::getResponse(XBeeResponse &response) const {

	response.setMsbLength(_response.getMsbLength());
	response.setLsbLength(_response.getLsbLength());
	response.setApiId(_response.getApiId());
	response.setFrameLength(_response.getFrameDataLength());

	response.setFrameData(_response.getFrameData());
}

void XBee::readPacketUntilAvailable() {
	while (!(getResponse().isAvailable() || getResponse().isError())) {
		// read some more
		readPacket(false);
	}
}

bool XBee::readPacket(int timeout, bool debug) {

	if (timeout < 0) {
		return false;
	}

	unsigned long start = millis();

	while (int((millis() - start)) < timeout) {

		readPacket(debug);

		if (getResponse().isAvailable()) {
			return true;
		} else if (getResponse().isError()) {
			return false;
		}
	}

	// timed out
	return false;
}

bool XBee::readPacketNoTimeout(bool debug) {
	readPacket(debug);

	if (getResponse().isAvailable()) {
		return true;
	} else if (getResponse().isError()) {
		Serial.print("Error ");
		Serial.println(getResponse().getErrorCode());
		return false;
	}
	return false;
}

void XBee::readPacket(bool debug) {
	// reset previous response
	if (_response.isAvailable() || _response.isError()) {
		// discard previous packet and start over
		resetResponse();
	}

	//_serial->print(available());
	while (available()) {

		b = read();
		if (debug)
			_serial->print((char) b);

		if (_pos > 0 && b == START_BYTE && ATAP == 2) {
			// new packet start before previous packeted completed -- discard previous packet and start over
			_response.setErrorCode(UNEXPECTED_START_BYTE);
			return;
		}

		if (_pos > 0 && b == ESCAPE) {
			if (available()) {
				b = read();
				if (debug)
					_serial->print((char) b);

				b = 0x20 ^ b;
			} else {
				// escape byte.  next byte will be
				_escape = true;
				continue;
			}
		}

		if (_escape == true) {
			b = 0x20 ^ b;
			_escape = false;
		}

		// checksum includes all bytes starting with api id
		if (_pos >= API_ID_INDEX) {
			_checksumTotal += b;
		}

		switch (_pos) {
			case 0:
				if (b == START_BYTE) {
					_pos++;
				}

				break;
			case 1:
				// length msb
				_response.setMsbLength(b);
				_pos++;

				break;
			case 2:
				// length lsb
				_response.setLsbLength(b);
				_pos++;

				break;
			case 3:
				_response.setApiId(b);
				_pos++;

				break;
			default:
				// starts at fifth byte
				//Serial.println(_pos);
				if (_pos > MAX_FRAME_DATA_SIZE) {
					// exceed max size.  should never occur
					_response.setErrorCode(PACKET_EXCEEDS_BYTE_ARRAY_LENGTH);
					return;
				}

				// check if we're at the end of the packet
				// packet length does not include start, length, or checksum bytes, so add 3
				if (_pos == (_response.getPacketLength() + 3)) {
					// verify checksum

					//std::cout << "read checksum " << static_cast<unsigned int>(b) << " at pos " << static_cast<unsigned int>(_pos) << std::endl;

					if ((_checksumTotal & 0xff) == 0xff) {
						_response.setChecksum(b);
						_response.setAvailable(true);

						_response.setErrorCode(NO_ERROR);
					} else {
						// checksum failed
						_response.setErrorCode(CHECKSUM_FAILURE);
					}

					// minus 4 because we start after start,msb,lsb,api and up to but not including checksum
					// e.g. if frame was one byte, _pos=4 would be the byte, pos=5 is the checksum, where end stop reading
					_response.setFrameLength(_pos - 4);

					// reset state vars
					_pos = 0;

					return;
				} else {
					// add to packet array, starting with the fourth byte of the apiFrame
					_response.getFrameData()[_pos - 4] = b;
					_pos++;
				}
		}
	}
}

// it's peanut butter jelly time!!

XBeeRequest::XBeeRequest(const uint8_t apiId, const uint8_t frameId) {
	_apiId = apiId;
	_frameId = frameId;
}

XBeeRequest::~XBeeRequest() {

}

void XBeeRequest::setFrameId(const uint8_t frameId) {
	_frameId = frameId;
}

uint8_t XBeeRequest::getFrameId() const {
	return _frameId;
}

uint8_t XBeeRequest::getApiId() const {
	return _apiId;
}

void XBeeRequest::setApiId(const uint8_t apiId) {
	_apiId = apiId;
}

//void XBeeRequest::reset() {
//	_frameId = DEFAULT_FRAME_ID;
//}

//uint8_t XBeeRequest::getPayloadOffset() {
//	return _payloadOffset;
//}
//
//uint8_t XBeeRequest::setPayloadOffset(uint8_t payloadOffset) {
//	_payloadOffset = payloadOffset;
//}

PayloadRequest::PayloadRequest(const uint8_t apiId, const uint8_t frameId, uint8_t *payload,
		const uint8_t payloadLength) :
		XBeeRequest(apiId, frameId) {
	_payloadPtr = payload;
	_payloadLength = payloadLength;
}

PayloadRequest::~PayloadRequest() {
}

uint8_t* PayloadRequest::getPayload() const {
	return _payloadPtr;
}

void PayloadRequest::setPayload(uint8_t* payload) {
	_payloadPtr = payload;
}

uint8_t PayloadRequest::getPayloadLength() const {
	return _payloadLength;
}

void PayloadRequest::setPayloadLength(const uint8_t payloadLength) {
	_payloadLength = payloadLength;
}

XBeeAddress::XBeeAddress() {

}

XBeeAddress64::XBeeAddress64() :
		XBeeAddress() {

}

XBeeAddress64::XBeeAddress64(const uint32_t msb, const uint32_t lsb) :
		XBeeAddress() {
	_msb = msb;
	_lsb = lsb;
}

uint32_t XBeeAddress64::getMsb() const {
	return _msb;
}

void XBeeAddress64::setMsb(const uint32_t msb) {
	_msb = msb;
}

uint32_t XBeeAddress64::getLsb() const {
	return _lsb;
}

void XBeeAddress64::setLsb(const uint32_t lsb) {
	_lsb = lsb;
}

bool XBeeAddress64::equals(const XBeeAddress64& x) const {
	if (_lsb == x._lsb && _msb == x._msb)
		return true;
	return false;

}

bool XBeeAddress64::operator<(const XBeeAddress64& addr) const {
	return ((_lsb < addr.getLsb()) && (addr.getLsb() > _lsb));
}

void XBeeAddress64::printAddress(Stream* _serial) const {

	_serial->print((char) ((_msb >> 24) & 0xff));
	_serial->print((char) ((_msb >> 16) & 0xff));
	_serial->print((char) ((_msb >> 8) & 0xff));
	_serial->print((char) (_msb & 0xff));
	_serial->print((char) ((_lsb >> 24) & 0xff));
	_serial->print((char) ((_lsb >> 16) & 0xff));
	_serial->print((char) ((_lsb >> 8) & 0xff));
	_serial->print((char) (_lsb & 0xff));
}

void XBeeAddress64::printAddressASCII(Stream* _serial) const {

	_serial->print(((_msb >> 24) & 0xff), HEX);
	_serial->print(((_msb >> 16) & 0xff), HEX);
	_serial->print(((_msb >> 8) & 0xff), HEX);
	_serial->print((_msb & 0xff), HEX);
	_serial->print(((_lsb >> 24) & 0xff), HEX);
	_serial->print(((_lsb >> 16) & 0xff), HEX);
	_serial->print(((_lsb >> 8) & 0xff), HEX);
	_serial->print((_lsb & 0xff), HEX);
}

Tx64Request::Tx64Request() :
		PayloadRequest(TX_64_REQUEST, DEFAULT_FRAME_ID, NULL, 0) {

}

Tx64Request::~Tx64Request() {

}

Tx64Request::Tx64Request(const XBeeAddress64 &addr64, const uint8_t option, uint8_t *data, const uint8_t dataLength,
		const uint8_t frameId) :
		PayloadRequest(TX_64_REQUEST, frameId, data, dataLength) {
	_addr64 = addr64;
	_option = option;
}

Tx64Request::Tx64Request(const XBeeAddress64 &addr64, uint8_t *data, const uint8_t dataLength) :
		PayloadRequest(TX_64_REQUEST, DEFAULT_FRAME_ID, data, dataLength) {
	_addr64 = addr64;
	_option = ACK_OPTION;
}

uint8_t Tx64Request::getFrameData(const uint8_t pos) const {

	if (pos == 0) {
		return (_addr64.getMsb() >> 24) & 0xff;
	} else if (pos == 1) {
		return (_addr64.getMsb() >> 16) & 0xff;
	} else if (pos == 2) {
		return (_addr64.getMsb() >> 8) & 0xff;
	} else if (pos == 3) {
		return _addr64.getMsb() & 0xff;
	} else if (pos == 4) {
		return (_addr64.getLsb() >> 24) & 0xff;
	} else if (pos == 5) {
		return (_addr64.getLsb() >> 16) & 0xff;
	} else if (pos == 6) {
		return (_addr64.getLsb() >> 8) & 0xff;
	} else if (pos == 7) {
		return _addr64.getLsb() & 0xff;
	} else if (pos == 8) {
		return _option;
	} else {
		return getPayload()[pos - TX_64_API_LENGTH];
	}
}

uint8_t Tx64Request::getFrameDataLength() const {
	return TX_64_API_LENGTH + getPayloadLength();
}

XBeeAddress64& Tx64Request::getAddress64() {
	return _addr64;
}

void Tx64Request::setAddress64(const XBeeAddress64& addr64) {
	_addr64 = addr64;
}

uint8_t Tx64Request::getOption() const {
	return _option;
}

void Tx64Request::setOption(const uint8_t option) {
	_option = option;
}

AtCommandRequest::AtCommandRequest() :
		XBeeRequest(AT_COMMAND_REQUEST, DEFAULT_FRAME_ID) {
	_command = NULL;
	clearCommandValue();
}

AtCommandRequest::AtCommandRequest(uint8_t *command, uint8_t *commandValue, uint8_t commandValueLength) :
		XBeeRequest(AT_COMMAND_REQUEST, DEFAULT_FRAME_ID) {
	_command = command;
	_commandValue = commandValue;
	_commandValueLength = commandValueLength;
}

AtCommandRequest::AtCommandRequest(uint8_t *command) :
		XBeeRequest(AT_COMMAND_REQUEST, DEFAULT_FRAME_ID) {
	_command = command;
	clearCommandValue();
}

AtCommandRequest::~AtCommandRequest() {

}

uint8_t* AtCommandRequest::getCommand() const {
	return _command;
}

uint8_t* AtCommandRequest::getCommandValue() const {
	return _commandValue;
}

uint8_t AtCommandRequest::getCommandValueLength() const {
	return _commandValueLength;
}

void AtCommandRequest::setCommand(uint8_t* command) {
	_command = command;
}

void AtCommandRequest::setCommandValue(uint8_t* value) {
	_commandValue = value;
}

void AtCommandRequest::setCommandValueLength(const uint8_t length) {
	_commandValueLength = length;
}

uint8_t AtCommandRequest::getFrameData(const uint8_t pos) const {

	if (pos == 0) {
		return _command[0];
	} else if (pos == 1) {
		return _command[1];
	} else {
		return _commandValue[pos - AT_COMMAND_API_LENGTH];
	}
}

void AtCommandRequest::clearCommandValue() {
	_commandValue = NULL;
	_commandValueLength = 0;
}

//void AtCommandRequest::reset() {
//	 XBeeRequest::reset();
//}

uint8_t AtCommandRequest::getFrameDataLength() const {
	// command is 2 byte + length of value
	return AT_COMMAND_API_LENGTH + _commandValueLength;
}

XBeeAddress64 RemoteAtCommandRequest::broadcastAddress64 = XBeeAddress64(0x0, BROADCAST_ADDRESS);

RemoteAtCommandRequest::RemoteAtCommandRequest() :
		AtCommandRequest(NULL, NULL, 0) {
	_remoteAddress16 = 0;
	_applyChanges = false;
	setApiId (REMOTE_AT_REQUEST);
}

RemoteAtCommandRequest::RemoteAtCommandRequest(uint16_t remoteAddress16, uint8_t *command, uint8_t *commandValue,
		uint8_t commandValueLength) :
		AtCommandRequest(command, commandValue, commandValueLength) {
	_remoteAddress64 = broadcastAddress64;
	_remoteAddress16 = remoteAddress16;
	_applyChanges = true;
	setApiId (REMOTE_AT_REQUEST);
}

RemoteAtCommandRequest::RemoteAtCommandRequest(uint16_t remoteAddress16, uint8_t *command) :
		AtCommandRequest(command, NULL, 0) {
	_remoteAddress64 = broadcastAddress64;
	_remoteAddress16 = remoteAddress16;
	_applyChanges = false;
	setApiId (REMOTE_AT_REQUEST);
}

RemoteAtCommandRequest::RemoteAtCommandRequest(XBeeAddress64 &remoteAddress64, uint8_t *command, uint8_t *commandValue,
		uint8_t commandValueLength) :
		AtCommandRequest(command, commandValue, commandValueLength) {
	_remoteAddress64 = remoteAddress64;
	// don't worry.. works for series 1 too!
	_remoteAddress16 = ZB_BROADCAST_ADDRESS;
	_applyChanges = true;
	setApiId (REMOTE_AT_REQUEST);
}

RemoteAtCommandRequest::RemoteAtCommandRequest(XBeeAddress64 &remoteAddress64, uint8_t *command) :
		AtCommandRequest(command, NULL, 0) {
	_remoteAddress64 = remoteAddress64;
	_remoteAddress16 = ZB_BROADCAST_ADDRESS;
	_applyChanges = false;
	setApiId (REMOTE_AT_REQUEST);
}

uint16_t RemoteAtCommandRequest::getRemoteAddress16() const {
	return _remoteAddress16;
}

void RemoteAtCommandRequest::setRemoteAddress16(const uint16_t remoteAddress16) {
	_remoteAddress16 = remoteAddress16;
}

XBeeAddress64& RemoteAtCommandRequest::getRemoteAddress64() {
	return _remoteAddress64;
}

void RemoteAtCommandRequest::setRemoteAddress64(const XBeeAddress64 &remoteAddress64) {
	_remoteAddress64 = remoteAddress64;
}

bool RemoteAtCommandRequest::getApplyChanges() const {
	return _applyChanges;
}

void RemoteAtCommandRequest::setApplyChanges(const bool applyChanges) {
	_applyChanges = applyChanges;
}

uint8_t RemoteAtCommandRequest::getFrameData(const uint8_t pos) const {
	if (pos == 0) {
		return (_remoteAddress64.getMsb() >> 24) & 0xff;
	} else if (pos == 1) {
		return (_remoteAddress64.getMsb() >> 16) & 0xff;
	} else if (pos == 2) {
		return (_remoteAddress64.getMsb() >> 8) & 0xff;
	} else if (pos == 3) {
		return _remoteAddress64.getMsb() & 0xff;
	} else if (pos == 4) {
		return (_remoteAddress64.getLsb() >> 24) & 0xff;
	} else if (pos == 5) {
		return (_remoteAddress64.getLsb() >> 16) & 0xff;
	} else if (pos == 6) {
		return (_remoteAddress64.getLsb() >> 8) & 0xff;
	} else if (pos == 7) {
		return _remoteAddress64.getLsb() & 0xff;
	} else if (pos == 8) {
		return (_remoteAddress16 >> 8) & 0xff;
	} else if (pos == 9) {
		return _remoteAddress16 & 0xff;
	} else if (pos == 10) {
		return _applyChanges ? 2 : 0;
	} else if (pos == 11) {
		return getCommand()[0];
	} else if (pos == 12) {
		return getCommand()[1];
	} else {
		return getCommandValue()[pos - REMOTE_AT_COMMAND_API_LENGTH];
	}
}

uint8_t RemoteAtCommandRequest::getFrameDataLength() const {
	return REMOTE_AT_COMMAND_API_LENGTH + getCommandValueLength();
}

void XBee::packageSend(const XBeeRequest &request) {
	uint8_t index = 0;
	uint8_t buffer[114];
	buffer[0] = START_BYTE;

	packageByte(buffer, START_BYTE, false, index);

	uint8_t msbLen = ((request.getFrameDataLength() + 2) >> 8) & 0xff;
	uint8_t lsbLen = (request.getFrameDataLength() + 2) & 0xff;
	packageByte(buffer, msbLen, true, index);
	packageByte(buffer, lsbLen, true, index);

	packageByte(buffer, request.getApiId(), true, index);
	packageByte(buffer, request.getFrameId(), true, index);

	uint8_t checksum = 0;

	// compute checksum, start at api id
	checksum += request.getApiId();
	checksum += request.getFrameId();

	for (int i = 0; i < request.getFrameDataLength(); i++) {
		packageByte(buffer, request.getFrameData(i), true, index);
		checksum += request.getFrameData(i);
	}

	// perform 2s complement
	checksum = 0xff - checksum;

	packageByte(buffer, checksum, true, index);

	Serial.write(buffer, index);
	//Serial.print("index");
	//Serial.println(index);
	flush();

}

void XBee::send(const XBeeRequest &request) {
	// the new new deal

	sendByte(START_BYTE, false);

	// send length
	uint8_t msbLen = ((request.getFrameDataLength() + 2) >> 8) & 0xff;
	uint8_t lsbLen = (request.getFrameDataLength() + 2) & 0xff;

	sendByte(msbLen, true);
	sendByte(lsbLen, true);

	// api id
	sendByte(request.getApiId(), true);
	sendByte(request.getFrameId(), true);

	uint8_t checksum = 0;

	// compute checksum, start at api id
	checksum += request.getApiId();
	checksum += request.getFrameId();

	//std::cout << "frame length is " << static_cast<unsigned int>(request.getFrameDataLength()) << std::endl;

	for (int i = 0; i < request.getFrameDataLength(); i++) {
//		std::cout << "sending byte [" << static_cast<unsigned int>(i) << "] " << std::endl;
		sendByte(request.getFrameData(i), true);
		checksum += request.getFrameData(i);
	}

	// perform 2s complement
	checksum = 0xff - checksum;

//	std::cout << "checksum is " << static_cast<unsigned int>(checksum) << std::endl;

	// send checksum
	sendByte(checksum, true);

	// send packet (Note: prior to Arduino 1.0 this flushed the incoming buffer, which of course was not so great)
	flush();
}

void XBee::sendByte(const uint8_t b, const bool escape) {

	if (escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF)) {
//		std::cout << "escaping byte [" << toHexString(b) << "] " << std::endl;
		write (ESCAPE);
		write(b ^ 0x20);
	} else {
		write(b);
	}
}

void XBee::packageByte(uint8_t buffer[], const uint8_t b, const bool escape, uint8_t &index) {

	if (escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF)) {
//		std::cout << "escaping byte [" << toHexString(b) << "] " << std::endl;
		buffer[index] = ESCAPE;
		index++;
		buffer[index] = b ^ 0x20;
	} else {
		buffer[index] = b;
	}
	index++;
}

void XBee::getMyAddress(XBeeAddress64& address, bool debug) {

	uint8_t shCmd[] = { 'S', 'H' };

	// serial low
	uint8_t slCmd[] = { 'S', 'L' };

	AtCommandRequest atCommandRequest = AtCommandRequest(shCmd);

	send(atCommandRequest);
	atCommandRequest.setCommand(slCmd);
	send(atCommandRequest);
	uint8_t notfound = 0;

	while (notfound < 2) {
		if (readPacket(1000, debug)) {

			// should be an AT command response
			if (getResponse().getApiId() == AT_COMMAND_RESPONSE) {
				AtCommandResponse atResponse = AtCommandResponse();
				getResponse().getAtCommandResponse(atResponse);

				if (atResponse.isOk()) {
					if (!notfound) {
						address.setMsb(
								(atResponse.getValue()[0] << 24) + (atResponse.getValue()[1] << 16)
										+ (atResponse.getValue()[2] << 8) + atResponse.getValue()[3]);
						++notfound;
					} else {
						address.setLsb(
								(atResponse.getValue()[0] << 24) + (atResponse.getValue()[1] << 16)
										+ (atResponse.getValue()[2] << 8) + atResponse.getValue()[3]);
						++notfound;
					}
				}
			}
		}
	}

	Serial.print("ThisAddress");
	address.printAddress(&Serial);
}
