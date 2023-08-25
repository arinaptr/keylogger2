from http.server import BaseHTTPRequestHandler, HTTPServer
import os
import socks
import socket
import requests

class FileUploadHandler(BaseHTTPRequestHandler):
    file_counter = {} 

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        uploaded_file = self.rfile.read(content_length)

        client_ip = self.client_address[0] 
        
        if client_ip in FileUploadHandler.file_counter:
            FileUploadHandler.file_counter[client_ip] += 1
        else:
            FileUploadHandler.file_counter[client_ip] = 1
        
        counter = FileUploadHandler.file_counter[client_ip]
        filename = os.path.join(os.path.expanduser("~/Desktop/kl_uploads"), f"{client_ip}_{counter}.exe")
        
        with open(filename, 'wb') as f:
            f.write(uploaded_file)

        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b'File uploaded successfully')

if __name__ == '__main__':
    server_class = HTTPServer
    handler_class = FileUploadHandler
    host = '127.0.0.1'
    port = 8080
    server_address = (host, port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting server on {host}:{port}")
    httpd.serve_forever()
