# cattun

Access a TUN device from cattun's stdin and stdout

This program allows full access to a TUN device through pipes. Datagrams are 
relayed nearly unchanged. The only thing that is added is a int-sized length 
at the beginning of the datagram. This length is the length of the datagram.
