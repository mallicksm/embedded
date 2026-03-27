#!/usr/bin/env python3

import argparse
import select
import socket
import sys
import termios
import time
import tty


def parse_args():
   parser = argparse.ArgumentParser(description="Simple QEMU UART TCP terminal")
   parser.add_argument("--host", default="localhost", help="QEMU UART host")
   parser.add_argument("--port", type=int, default=4444, help="QEMU UART TCP port")
   return parser.parse_args()

def connect_with_retry(host: str, port: int) -> socket.socket:
   while True:
      try:
         sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         sock.connect((host, port))
         sock.setblocking(False)
         return sock
      except ConnectionRefusedError:
         try:
            time.sleep(0.1)
         except KeyboardInterrupt:
            print("\nExiting.")
            sys.exit(0)

def main():
   args = parse_args()

   # 👇 print BEFORE raw mode so formatting is normal
   print(f"Connected to UART at {args.host}:{args.port}")
   print("Ctrl-C to quit.\n", flush=True)

   sock = connect_with_retry(args.host, args.port)

   stdin_fd = sys.stdin.fileno()
   old_settings = termios.tcgetattr(stdin_fd)

   try:
      tty.setraw(stdin_fd)

      while True:
         read_list, _, _ = select.select([sock, sys.stdin], [], [])

         if sock in read_list:
            try:
               data = sock.recv(4096)
            except BlockingIOError:
               data = b""

            if not data:
               print("\nConnection closed by remote.")
               break

            text = data.decode(errors="ignore").replace("\n", "\r\n")
            sys.stdout.write(text)
            sys.stdout.flush()

         if sys.stdin in read_list:
            data = sys.stdin.buffer.read(4096)
            if data:
               if b"\x03" in data:  # Ctrl-C
                  print("\nExiting.")
                  break
               sock.sendall(data)

   finally:
      termios.tcsetattr(stdin_fd, termios.TCSADRAIN, old_settings)
      sock.close()


if __name__ == "__main__":
   main()
