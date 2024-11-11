require 'socket'

socket = TCPSocket.new 'localhost', 8080

socket.write("GET /index.html")

socket.close