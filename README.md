# Chronode

A modern, header-only, in-source C++17 profiling library. The name is
derived from the fact that timing/measurement information is stored
in a graph structure, where each timer (Node) is aware of it's own
absolute time and the time relative to a parent.

# Example (Pure C++)

```c++
// Soon!
```

# Example (C++ & Macros)

```c++
// Soon!
```

# UI

Eventually, the Chronode JSON output will be viewable in either the pure
web-based visualizer OR the web visualizer embedded/packaged in Electron; the
trick will be streaming the JSON itself to the frontend. With the Electron
version, it will be as easy as using a named pipe.

# TODO

- There is a bug in Node where it always thinks the number of ACTIVE children is
  however large the deque is; instead, it should be whatever the value of `_c`
  is (unless we ditch the `reset()` mechanism altogether).
- Implement better error handling/checking (prevent "reporting" when
  any Node isn't properly stopped, etc).
- Some kind of vector-based (SVG, HTML5 Canvas, Cairo) reporting display.
- In the `Profile` object, add some kind of mechanism for tracking/alerting
  large changes/variations from the accumulated average.
- Use an optimized "ring buffer" in Profile.

# MISC (Don't Forget)

```bash
npm init -y
npm install electron --save-dev
npm install electron-packager --save-dev
# Edit package.json and add:
#	"start": "electron main.js",
#	"package": "electron-packager . --platform=linux --arch=x64 --out=dist --overwrite"
npm start
npm run package
```

```cpp
#include <uvw.hpp>

int main() {
	auto loop = uvw::Loop::getDefault();
	auto tcp = loop->resource<uvw::PipeHandle>();

	// Simulate creating JSON data
	std::string jsonData = R"({"message": "Hello from C++ CLI!"})";

	tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent&, uvw::PipeHandle& handle) {
		handle.close();
	});

	tcp->connect("communication_pipe");
	tcp->once<uvw::ConnectEvent>([&jsonData](const uvw::ConnectEvent&, uvw::PipeHandle& handle) {
		handle.write(std::make_unique<std::string>(jsonData));
		handle.close();
	});

	loop->run();

	return 0;
}
```

```javascript
const net = require('net');
const fs = require('fs');

const pipeName = process.platform === 'win32' ? '\\\\.\\pipe\\communication_pipe' : 'communication_pipe';

// Create a server listening on the named pipe
const server = net.createServer((socket) => {
	console.log('C++ connected to the named pipe');

	// Handle incoming data
	socket.on('data', (data) => {
		const jsonData = data.toString();
		console.log(`Received JSON data from C++: ${jsonData}`);
	});

	// Handle socket closure
	socket.on('end', () => {
		console.log('C++ disconnected');
	});
});

// Start the server
server.listen(pipeName, () => {
	console.log('Node.js server is listening on the named pipe');
});

// Clean up on process exit
process.on('exit', () => {
	try {
		fs.unlinkSync(pipeName);
	} catch (err) {
		// Ignore errors if the file doesn't exist
	}
});
```
