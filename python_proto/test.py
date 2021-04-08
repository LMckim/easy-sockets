from easysocket import EasySocket_Client, Protocol

sock_tcp = EasySocket_Client()
sock_tcp.connect('localhost', 6555)
sock_tcp.send("Hello World!")

sock_udp = EasySocket_Client()
sock_udp.protocol(Protocol.UDP)
sock_udp.connect('localhost', 6556)
sock_udp.send("Hello Again!!")