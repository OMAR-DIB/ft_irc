#!/usr/bin/env python3
"""
Simple IRC client test script for ft_irc server
Usage: python3 test_client.py [port] [password]
"""

import socket
import sys
import time

def test_irc_server(host='localhost', port=6668, password='mypass123'):
    print(f"Connecting to IRC server at {host}:{port}")
    
    try:
        # Create socket and connect
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print("✓ Connected to server")
        
        def send_command(cmd):
            print(f"→ {cmd.strip()}")
            sock.send((cmd + '\r\n').encode())
            time.sleep(0.1)
        
        def recv_response():
            try:
                data = sock.recv(1024).decode()
                if data:
                    for line in data.strip().split('\n'):
                        if line.strip():
                            print(f"← {line.strip()}")
                return data
            except:
                return ""
        
        # Test authentication sequence
        print("\n=== Testing Authentication ===")
        send_command(f"PASS {password}")
        recv_response()
        
        send_command("NICK testuser")
        recv_response()
        
        send_command("USER testuser 0 * :Test User")
        recv_response()
        time.sleep(0.2)  # Wait for welcome messages
        recv_response()
        
        # Test channel operations
        print("\n=== Testing Channel Operations ===")
        send_command("JOIN #test")
        recv_response()
        
        send_command("PRIVMSG #test :Hello, World!")
        recv_response()
        
        send_command("TOPIC #test :This is a test topic")
        recv_response()
        
        send_command("MODE #test")
        recv_response()
        
        print("\n=== Test Complete ===")
        print("✓ IRC server is working correctly!")
        
    except ConnectionRefusedError:
        print(f"✗ Could not connect to {host}:{port}")
        print("Make sure the IRC server is running with: ./ircserv 6668 mypass123")
    except Exception as e:
        print(f"✗ Error: {e}")
    finally:
        try:
            sock.close()
        except:
            pass

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 6668
    password = sys.argv[2] if len(sys.argv) > 2 else "mypass123"
    test_irc_server(port=port, password=password)