# Producer-Consumer Problem Solution

## Overview
This project presents a solution to the classic Producer-Consumer problem using Linux-based message queues, System V semaphores, shared memory, and file I/O. It addresses the challenge of synchronizing two independent processes, a producer and a consumer, to efficiently manage a finite circular buffer.

## Project Structure
The project is organized as follows:

1. **Message Queue and Semaphore Implementation**: Utilizes System V semaphores for process synchronization and message queues for client-server communication.

2. **Request-Response Mechanism**: Implements a Request-Response system between the client and server for acquiring keys used in semaphore and shared memory creation.

3. **Shared Memory**: Utilizes shared memory to manage a circular buffer containing 10 buffers, each with a string of 1024 bytes, a sequence number, and a count to track data insertion.

4. **Producer**: Reads data from a file and writes it into the shared memory buffers, maintaining proper synchronization with the consumer.

5. **Consumer**: Reads data from shared memory, validates sequence numbers, writes to an output file, and verifies data consistency.

## Getting Started
To run the project, follow these steps:

1. Compile the code: Use your preferred compiler to compile the producer and consumer programs separately.

2. Execute the Producer: Run the producer program, specifying the input file containing the data to be processed.

3. Execute the Consumer: Run the consumer program, specifying the output file where the processed data will be stored.

4. Check Results: Verify the output file for correctness and ensure that data consistency is maintained.

## Requirements
- Linux-based operating system.
- C/C++ compiler.
- System V semaphore and message queue libraries.

## Notes
- This project focuses on providing a clear solution to the Producer-Consumer problem and may not be suitable for production use without further enhancements and error handling.

## License
This project is released under the [MIT License](LICENSE.md).
