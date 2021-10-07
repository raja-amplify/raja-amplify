#include <Regexp.h>
char requestedURL [200] = "ws://ec2-65-0-126-105.ap-south-1.compute.amazonaws.com:8010/OCPP16/5c866e81a2d9593de43efdb4/612b216b9a794d037695ad7f/amp04";
//char requestedURL [200] = "https://evcp.evsecerebro.com:485/websocket/amplify/jarvis";
char port[]="";
  int portNo = 0;
  char host[]="";
  int protocol=0;
  char pt[10];
  char hostName[200];
      char pagePartOfURL[200];
      char domain[200];
      char buf[200];
      bool portFound = false;
      
void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  
 /* char pws[10]="ws";
  char pwss[10]="wss";
  char phttp[10]="http";
  char phttps[10]="https";*/

//Regex object creation.
// match state object
  MatchState *ms; // It is always advisible to use pointer for objects as we can free memory later.

  ms = new MatchState();

/*  int n = ws_url_prefix_m.length();
 
    // declaring character array
    char requestedURL[n + 1];
 
    // copying the contents of the
    // string to char array
    strcpy(requestedURL, ws_url_prefix_m.c_str());
*/
      strcpy(buf, requestedURL);
      // Now, find the spot where the domain/server ends and the rest of the URL starts
      int splitLocation = 0;
      boolean foundSplit = false;
     int length = sizeof(requestedURL);
      // Go through the terminal input, one character at a time and check for a forward-slash
      // if found, make sure the character before or after is not a forward-slash also, because
      // in that case we are dealing with the http:// part of a url.
      // if we find a single forward-slash, remember the position where that is in the char[]
      for (int i=0; i < length; i++) {
        if (foundSplit != true && requestedURL[i] == '/') {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
            Serial.println("First split");
          }
        }
      }
      
      // if foundSplit is true, then requestedURL can be split, if false, there is only a domain/server and no page
      // now store the domain/server part in hostName[]
      if (foundSplit == true) {
        for (int j=0; j < splitLocation; j++) {
          hostName[j] = requestedURL[j];
        }
        
        // and the rest in pagePartOfURL[]
        for (int k=splitLocation; k < length; k++) {
          pagePartOfURL[k-splitLocation] = requestedURL[k];
        }
        hostName[splitLocation] = '\0';
        pagePartOfURL[length-splitLocation] = '\0';
      }  

      Serial.println("Extracting");
      Serial.println(hostName);
      Serial.println(pagePartOfURL);
      Serial.println("Extracted");

      /*
       * Logic for extracting the port number and the protocol
       */

    ms->Target (buf);  // set its address
  //Serial.println (buf);

  char wsscheck = ms->Match ("wss://");
  char wscheck = ms->Match ("ws://");
  char httpcheck = ms->Match ("http://");
  char httpscheck = ms->Match ("https://");


  //Serial.println("Checking for protocol:");
  if(wsscheck > 0)
    {
      Serial.print("protocol = ");
    //  strcpy(protocol,pwss);
    protocol = 0;
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("wss://", "");
    Serial.print("host = ");
    Serial.println(domain);

     }
    else if(wscheck > 0)
    {
      Serial.print("protocol = ");
      //char protocol[] = "ws";
      protocol = 1;
      //strcpy(protocol,pws);
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("ws://", "");
    Serial.print("host = ");
    Serial.println(domain);
    }
    else if(httpcheck > 0)
    {
      Serial.print("protocol = ");
     // strcpy(protocol,phttp);
     protocol = 2;
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("http://", "");
    Serial.print("host = ");
    Serial.println(domain);

    }
    else if (httpscheck > 0)
    {
      Serial.print("protocol = ");
      protocol = 3;
      //strcpy(protocol,phttps);
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("https://", "");
    Serial.print("host = ");
    Serial.println(domain);

    } 

/*
 * Logic for port number
 */
 ms->Target (buf);  // set its address
 char portCheck = ms->Match(":.(%d+)"); // first match of anything after : gives the port 
if (portCheck > 0)
    {
    strcpy(port,ms->GetMatch (buf));
    ms->Target(port);
    ms->GlobalReplace (":", "");
    portNo = atoi(port);
    Serial.print("port = ");
    Serial.println(port);
    portFound = true;
    }
  else
  {
    portFound = false;

    if(httpcheck >0)
    {
      strcpy(port,"80");
      portNo = 80;
    }
    else if(wscheck > 0)
    {
      strcpy(port,"80");
      portNo = 80;
    }
    else if (httpscheck >0 )
    {
      strcpy(port,"443");
      portNo = 443;
    }
    else if (wsscheck > 0 )
    {
      strcpy(port,"443");
      portNo = 443;
    }
    
    Serial.print("port = ");
    Serial.println(portNo);
  }

/************
 * logic for domain name
 */
foundSplit = false;
//bool portFound = true;
for (int i=0; i < length; i++) {
        if (foundSplit != true && requestedURL[i] == ':' && portFound == true) {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
          }
        }

        else if (foundSplit != true && requestedURL[i] == '/' && portFound == false) {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
          }
        }
      }

      if (foundSplit == true) {
        for (int j=0; j < splitLocation; j++) {
          domain[j] = requestedURL[j];
        }
        domain[splitLocation] = '\0';
        Serial.print("Domain = ");
        Serial.println(domain); 
      }





  /*
   * Values at the end of scraping
   */

Serial.println("Values at the end of scraping");

switch (protocol)
{
  case 0:
          strcpy(pt,"wss");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("wss://", "");
          
          break;
  case 1:
          strcpy(pt,"ws");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("ws://", "");
          
          break;
  case 2:
          strcpy(pt,"http");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("http://", "");
          
          break;
  case 3:
          strcpy(pt,"https");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("https://", "");
          break;
  default:
          Serial.println("Undefined protocol");
}

    Serial.print("port = ");
    Serial.println(portNo);
    Serial.print("domain = ");
    Serial.println(domain);
    Serial.print("protocol = ");
    Serial.println(pt);
    Serial.print("context root = ");
    Serial.println(pagePartOfURL);

      
}

void loop() {
  // put your main code here, to run repeatedly:

}
