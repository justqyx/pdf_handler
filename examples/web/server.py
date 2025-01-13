#!/usr/bin/env python3
from http.server import HTTPServer, SimpleHTTPRequestHandler
import os

class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        SimpleHTTPRequestHandler.end_headers(self)

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

if __name__ == '__main__':
    # Change to the project root directory
    os.chdir(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
    
    port = 8000
    print(f"Starting server at http://localhost:{port}")
    print(f"Open http://localhost:{port}/examples/web/ in your browser")
    httpd = HTTPServer(('localhost', port), CORSRequestHandler)
    httpd.serve_forever()
