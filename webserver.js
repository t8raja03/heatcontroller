var http = require('http').createServer(handler); //require http server, and create server with function handler()
var fs = require('fs'); //require filesystem module
var io = require('socket.io')(http)
const SerialPort = require('serialport')
const port = new SerialPort('/dev/ttyACM0', {
	baudRate: 9600,
	dataBits: 8,
	parity: 'none',
	stopBits: 1
})

http.listen(8080); //listen to port 8080

var voltData = ''; 							// this stores the clean data
var tempData = ''; 							// this stores the clean data
var readData = '';  							// this stores the buffer
var modeData = '';
port.on('data', function (data) { 					// call back when data is received
    readData += data.toString(); 					// append data to buffer

    if (readData.indexOf('S') >= 0 && readData.indexOf('V') >=0 && readData.indexOf('T') >= 0 && readData.indexOf('X') >= 0) {
	modeData = readData.substring(readData.indexOf('S') + 1, readData.indexOf('V'));
        voltData = readData.substring(readData.indexOf('V') + 1, readData.indexOf('T'));
        tempData = readData.substring(readData.indexOf('T') + 1, readData.indexOf('X'));
        readData = '';
        
	io.sockets.emit('voltage_received', voltData);
	io.sockets.emit('temperature_received', tempData);
	io.sockets.emit('mode_received', modeData);
	console.log("Received data through serial: Mode="+modeData.toString()+" Voltage="+voltData.toString()+" Temperature="+tempData.toString());
    }

});


function handler (req, res) { //create server
  	fs.readFile(__dirname + '/public/index.html', function(err, data) { 		//read file index.html in public folder
    		if (err) {
      			res.writeHead(404, {'Content-Type': 'text/html'}); 		//display 404 on error
      			return res.end("404 Not Found");
    		}
    		res.writeHead(200, {'Content-Type': 'text/html'}); 			//write HTML
    		res.write(data); 							//write data from index.html
    		return res.end();
  	});
}

io.sockets.on('connection', function (socket) {						// WebSocket Connection
  	var mains = 0; 								//static variable for current status
  	socket.on('main', function(data) { 						//get light switch status from client
		mains = data;
		if (mains == '1') {
			port.write('led 1', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
			port.write('main1', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
		}
		else if (mains == '2') {
			port.write('led 1', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
			port.write('main2', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
		}
		else if (mains == '3') {
			port.write('led 1', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
			port.write('main3', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
		}
		else {
			port.write('led 0', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
			port.write('main0', function(err) {
				if (err) {
					return console.log('Error on write: ', err.message)
				}
			})
		}
			
	});

	socket.on('data_received', function (msg) {
       		 console.log(msg);
    	});
		
});
