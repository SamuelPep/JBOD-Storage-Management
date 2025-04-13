
⸻

JBOD Networking Client 

Submitted by: Samuel Peprah Millner

JBOD Networking Client is a C-based client-side implementation of a custom network protocol that communicates with a JBOD (Just a Bunch Of Disks) server. It builds and sends structured packets to simulate remote disk operations over a TCP/IP socket connection.

Time spent: ~8–10 hours total (design, implementation, and debugging)

⸻

✅ Required Features

The following required functionality was implemented and tested:
	•	jbod_connect() establishes a TCP socket connection to the JBOD server.
	•	jbod_disconnect() cleanly closes the connection.
	•	send_packet() wraps opcode + block into a 261-byte network packet and sends it to the server.
	•	recv_packet() receives a response and extracts the return code and block (if present).
	•	jbod_client_operation() replaces local jbod_operation() and handles full request-response exchange.
	•	Able to pass all trace file tests (simple-input, random-input, linear-input) with correct output.

⸻


⸻

🔧 Technical Overview

This project extended an existing storage emulator by enabling network-based disk communication using a custom protocol. A full packet consists of:
	•	4 bytes — Opcode
	•	1 byte — Info code (bit flags: return value + block presence)
	•	256 bytes — Data block payload (optional)

The client performs the following steps per request:
	1.	Builds a packet_t struct in memory (with optional block data).
	2.	Sends the packet using nwrite().
	3.	Waits for and reads a full 261-byte response using nread().
	4.	Extracts the return value and optionally the data block into a buffer.

⸻

💡 Key Concepts Practiced
	•	C socket programming (TCP connect, send, receive)
	•	Byte-level data structuring (serialization/deserialization)
	•	Bitwise flag manipulation
	•	Replacing function calls with network logic (jbod_operation → jbod_client_operation)
	•	Pointer-based memory handling with malloc, memcpy

⸻

🧠 Notes

Challenges Faced:
	•	Understanding why a 261-byte packet structure was required and how to build it correctly.
	•	Correctly managing pointer arithmetic and memory allocation (especially when casting uint8_t*).
	•	Learning how to use bitwise operations to pack and unpack the info_code byte.
	•	Ensuring reliable transmission and reception of data over sockets using nwrite() and nread().

⸻

🧪 Testing

Testing was performed by running the JBOD server (./jbod_server -v) and then executing the tester with different trace files. Output was verified using:

diff output.txt traces/random-expected-output



