/*
ISS LAMP

This sketch gets ISS data from a python script on robottobox
using an Arduino Wiznet Ethernet shield.

and ALSO NTP time from the danish NTP pool, with dns lookup.

Todo:
VFD to class .. or not.. 800 lines of code isn't that bad.. is it?

*/
#include <SPI.h>
#include <Ethernet.h>
#include <Dns.h>

byte mac[] = {  0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; //MAC address of the Ethernet shield

IPAddress DNS_IP(8,8,8,8); //Google DNS.

char hostName[] = "bropdox.moore.dk"; //should init with a null-char termination.
IPAddress bropdoxIP;//init empty IP adress container

// Initialize the Ethernet client libraries
EthernetClient client;
DNSClient my_dns; //for dns lookup of NTP server
EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP


//UDP (robottobox) stuff:
const unsigned int localPort = 31337;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 16; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE ]; //buffer to hold incoming and outgoing packets
int UDPretryDelay = 0;
int UDPretries = 0;

unsigned int likes=0; //number of new likes

void setup() {

    // start the Ethernet connection:
    if (Ethernet.begin(mac) == 0) while(1); //dead end.
    else
    {
        String localIpString="  "; //really weird.. string needs to be initialized with something in it to work properly...
        localIpString=String(Ethernet.localIP()[0]); //first byte

        //formatting for print:
        for(byte ip=1;ip<4;ip++){localIpString+='.'; localIpString+=String(Ethernet.localIP()[ip]);} //last 3 bytes
        Serial.print("IP: ");
    	Serial.println(localIpString);
        delay(1500);
    }

    Udp.begin(localPort);
    delay(1000); //give the ethernet shield some time.

    my_dns.begin(DNS_IP);

    lookup_ip();

    delay(500);

    //transform IP byte array to string so that the debug window can tell user what ip is being used:
    static char ipstringbuf[16];
    sprintf(ipstringbuf, "%d.%d.%d.%d\0", bropdoxIP[0], bropdoxIP[1], bropdoxIP[2], bropdoxIP[3]);
    Serial.print("bropdox IP resolved: ");
    Serial.println(ipstringbuf);

    delay(500);

    Serial.println("UDP TX -> bropdox");

    sendUDPpacket(bropdoxIP);

    UDPwait(); //false = NTP

    Serial.print("NTP RX: ");

    handleUDP();

    delay(500);

}


/*
LLLLLLLLLLL
L:::::::::L
L:::::::::L
LL:::::::LL
  L:::::L                  ooooooooooo      ooooooooooo   ppppp   ppppppppp
  L:::::L                oo:::::::::::oo  oo:::::::::::oo p::::ppp:::::::::p   ::::::
  L:::::L               o:::::::::::::::oo:::::::::::::::op:::::::::::::::::p  ::::::
  L:::::L               o:::::ooooo:::::oo:::::ooooo:::::opp::::::ppppp::::::p ::::::
  L:::::L               o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
  L:::::L               o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
  L:::::L               o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
  L:::::L         LLLLLLo::::o     o::::oo::::o     o::::o p:::::p    p::::::p ::::::
LL:::::::LLLLLLLLL:::::Lo:::::ooooo:::::oo:::::ooooo:::::o p:::::ppppp:::::::p ::::::
L::::::::::::::::::::::Lo:::::::::::::::oo:::::::::::::::o p::::::::::::::::p  ::::::
L::::::::::::::::::::::L oo:::::::::::oo  oo:::::::::::oo  p::::::::::::::pp
LLLLLLLLLLLLLLLLLLLLLLLL   ooooooooooo      ooooooooooo    p::::::pppppppp
                                                           p:::::p
                                                           p:::::p
                                                          p:::::::p
                                                          p:::::::p
                                                          p:::::::p
                                                          ppppppppp
*/

void loop()
{



}



void lookup_ip()
{
    if(my_dns.getHostByName(hostName, bropdoxIP) !=1)
    {
        Serial.println("DNS lookup failed");
        while(1); //dead end
    }
}

void(* resetFunc) () = 0; //declare reset function @ address 0

void UDPwait(boolean) //true if ISS, false if NTP.
{
while (!Udp.parsePacket())
  {
  delay(50);
  UDPretryDelay++;
  if (UDPretryDelay==300)  //if 15 seconds has passed without an answer
    {
      sendUDPpacket(bropdoxIP);
      UDPretries++;
      UDPretryDelay=0;
//      VFD.sendChar('.');
    }
  if(UDPretries==10)
    {
      Serial.println("No UDP RX for 50+sec, giving up.");
      while(1); //dead end
    }
  }
UDPretries=0;
UDPretryDelay=0;
}


/*
IIIIIIIIII   SSSSSSSSSSSSSSS    SSSSSSSSSSSSSSS      TTTTTTTTTTTTTTTTTTTTTTTXXXXXXX       XXXXXXX
I::::::::I SS:::::::::::::::S SS:::::::::::::::S     T:::::::::::::::::::::TX:::::X       X:::::X
I::::::::IS:::::SSSSSS::::::SS:::::SSSSSS::::::S     T:::::::::::::::::::::TX:::::X       X:::::X
II::::::IIS:::::S     SSSSSSSS:::::S     SSSSSSS     T:::::TT:::::::TT:::::TX::::::X     X::::::X
  I::::I  S:::::S            S:::::S                 TTTTTT  T:::::T  TTTTTTXXX:::::X   X:::::XXX
  I::::I  S:::::S            S:::::S                         T:::::T           X:::::X X:::::X    ::::::
  I::::I   S::::SSSS          S::::SSSS                      T:::::T            X:::::X:::::X     ::::::
  I::::I    SS::::::SSSSS      SS::::::SSSSS                 T:::::T             X:::::::::X      ::::::
  I::::I      SSS::::::::SS      SSS::::::::SS               T:::::T             X:::::::::X
  I::::I         SSSSSS::::S        SSSSSS::::S              T:::::T            X:::::X:::::X
  I::::I              S:::::S            S:::::S             T:::::T           X:::::X X:::::X
  I::::I              S:::::S            S:::::S             T:::::T        XXX:::::X   X:::::XXX ::::::
II::::::IISSSSSSS     S:::::SSSSSSSS     S:::::S           TT:::::::TT      X::::::X     X::::::X ::::::
I::::::::IS::::::SSSSSS:::::SS::::::SSSSSS:::::S           T:::::::::T      X:::::X       X:::::X ::::::
I::::::::IS:::::::::::::::SS S:::::::::::::::SS            T:::::::::T      X:::::X       X:::::X
IIIIIIIIII SSSSSSSSSSSSSSS    SSSSSSSSSSSSSSS              TTTTTTTTTTT      XXXXXXX       XXXXXXX
*/

unsigned long sendUDPpacket(IPAddress& address)
{
  // set 4 bytes in the buffer to 0

  memset(packetBuffer, 0, 5);


  packetBuffer[0] = '3';
  packetBuffer[1] = '1';
  packetBuffer[2] = '3';
  packetBuffer[3] = '7';
  packetBuffer[4] = '7';

  Udp.beginPacket(address, 31337); //remote port: 31337
  Udp.write(packetBuffer,5); //push the 5 bytes
  Udp.endPacket();
}


void handleUDP()
{
    memset(packetBuffer, 0, NTP_PACKET_SIZE); //reset packet buffer
    int read_bytes=Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
    likes=(unsigned int)atoi(packetBuffer[0]);
 }