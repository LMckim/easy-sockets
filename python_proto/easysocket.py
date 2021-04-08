from enum import Enum
from socket import (
    AF_INET, AF_INET6, AF_UNIX, 
    SOCK_DGRAM, SOCK_STREAM,
    SOL_SOCKET,
    SO_REUSEADDR, 
    SO_RCVTIMEO,
    SO_SNDTIMEO,
    socket
)

class Domain(Enum):
    UNIX = AF_UNIX
    IPV4 = AF_INET
    IPV6 = AF_INET6

class Protocol(Enum):
    TCP = SOCK_STREAM
    UDP = SOCK_DGRAM

class EasySocket:
    def __init__(self):
        self._fd:socket = None
        self._connected:bool = False

        self._host:str = None
        self._port:str = None

        self._domain = Domain.IPV4
        self._protocol = Protocol.TCP

        self._reusable = True
        self._send_timeout = 0
        self._recv_timeout = 0

    def __del__(self):
        self.close()
        
    def connect(self, host:str, port): 
        self._host = host
        self._port = port

    def close(self):
        if self._connected:
            self._connected = False
            self._fd.close()


    def domain(self, set_domain:Domain=Domain.IPV4): 
        if set_domain not in Domain: raise Exception("Domain not recognized")
        self._domain = set_domain

    def protocol(self, set_protocol:Protocol=Protocol.TCP): 
        if set_protocol not in Protocol: raise Exception("Protocol not recognized")
        self._protocol = set_protocol

    def send(self): 
        if not self._connected:
            raise Exception("Socket is not connected")
    def recv(self): 
        if not self._connected:
            raise Exception("Socket is not connected")

class EasySocket_Client(EasySocket):
    def __init__(self): super().__init__()

    def connect(self, host:str, port):
        super().connect(host, port)
        if self._protocol == Protocol.TCP:
            try:
                self._fd = socket(self._domain.value, self._protocol.value)
                self._fd.connect((host, port))
                if self._reusable: self._fd.setsockopt(SOL_SOCKET, SO_REUSEADDR, True)
            except Exception as e:
                pass # TODO
        elif self._protocol == Protocol.UDP:
            try:
                self._fd = socket(self._domain.value, self._protocol.value)
                self._fd.bind((host, port))
            except Exception as e:
                pass # TODO
            
        self._connected = True

    def send(self, data, destination=None):
        super().send()
        if not isinstance(data, bytes):
            if isinstance(data, str): data = bytes(data if data[len(data)-1] == '\n' else data + '\n', 'utf8')
            elif isinstance(data, (int, float)): data = bytes(data)
        if not self._connected: 
            print("nconn")
            pass

        if self._protocol == Protocol.TCP:
            self._fd.send(data)
        elif self._protocol == Protocol.UDP:
            if destination is None and self._host is None and self._port == None:
                raise Exception("Destination must be sent for UDP")
            print("send")
            self._fd.sendto(data, destination if destination is not None else (self._host, self._port))


        