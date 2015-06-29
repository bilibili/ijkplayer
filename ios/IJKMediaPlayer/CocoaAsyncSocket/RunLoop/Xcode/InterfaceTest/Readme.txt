InterfaceTest is an iPhone project that demonstrates some of AsyncSocket's interface abilities.

Most machines have multiple network interfaces. For example:
- Ethernet
- WiFi
- 3G (cellular)
- Bluetooth
- Loopback (local machine only)

Which leads to the question:
If you are setting up sockets, which interface will your socket be running on?

By default, if you don't specify an inteface, then the following rules generally apply:

- Server sockets (listening/accepting) will accept incoming connections on any inteface.
- Client sockets (connecting) will make outgoing connections on the primary interface.

The primary interface on iPhone is WiFi.
The primary interface on Mac is configurable via system preferences.

But you can specify a particular interface using AsyncSocket.  There are various examples of when one might want to do this.

- You're trying to make a bluetooth connection.

- Your iPhone app only supports WiFi, so you need to ensure your socket is only using the WiFi interface.

- You want to create a local-only server socket on Mac for inter-process communication. You want to ensure only processes running on the local device can connect to your server.


The IntefaceTest project demonstrates specifying a particular interface to use for an outgoing connection to google.com. You can force either the WiFi interface or the 3G/cellular interface.