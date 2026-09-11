#!/usr/bin/env python3
"""Loopback-only local server, serving just the public web app."""
import functools
import http.server
import pathlib
import socketserver
import threading
import webbrowser

root = pathlib.Path(__file__).resolve().parent / 'dist'
class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True
handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=str(root))
if __name__ == '__main__':
    with Server(('127.0.0.1', 8765), handler) as server:
        print('BREAD is running at http://localhost:8765 — Ctrl+C to stop.', flush=True)
        threading.Timer(0.5, lambda: webbrowser.open('http://localhost:8765')).start()
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
